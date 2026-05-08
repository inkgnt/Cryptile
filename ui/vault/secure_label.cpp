#include "secure_label.h"

#include <QPainter>
#include <QFile>
#include <QEvent>
#include <QDebug>
#include <sodium.h>
#include <QLabel>
#include <cmath>

static uint32_t nextUtf8Codepoint(const uint8_t*& ptr, const uint8_t* end) {
    if (ptr >= end) return 0;

    uint8_t byte0 = *ptr++;
    if (byte0 < 0x80) return byte0; // ASCII

    uint32_t codepoint = 0;
    int bytesToRead = 0;

    if ((byte0 & 0xE0) == 0xC0)      { codepoint = byte0 & 0x1F; bytesToRead = 1; }
    else if ((byte0 & 0xF0) == 0xE0) { codepoint = byte0 & 0x0F; bytesToRead = 2; }
    else if ((byte0 & 0xF8) == 0xF0) { codepoint = byte0 & 0x07; bytesToRead = 3; }
    else return '?'; // incorrect UTF-8

    while (bytesToRead > 0 && ptr < end) {
        uint8_t byte = *ptr++;
        if ((byte & 0xC0) != 0x80) return '?';
        codepoint = (codepoint << 6) | (byte & 0x3F);
        bytesToRead--;
    }
    return codepoint;
}

SecureLabel::SecureLabel(QWidget *parent)
    : QFrame(parent)
{
    // QFrame deafults QLabel like
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);

    loadFont();
}

SecureLabel::~SecureLabel() {
    // ~SecureBuffer is called
}

void SecureLabel::loadFont() {
    QFile fontFile(":/fonts/RobotoMono-Regular");
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qWarning() << "SecureLabel: Failed to load font!";
        return;
    }

    m_fontData = fontFile.readAll();
    const unsigned char* fontPtr = reinterpret_cast<const unsigned char*>(m_fontData.constData());

    if (stbtt_InitFont(&m_fontInfo, fontPtr, stbtt_GetFontOffsetForIndex(fontPtr, 0))) {
        m_fontLoaded = true;
    } else {
        qWarning() << "SecureLabel: stbtt_InitFont failed!";
    }
}

void SecureLabel::setSecureText(const uint8_t* utf8_data, size_t size) {
    m_textData = utf8_data;
    m_textSize = size;

    renderTextToPixels();

    updateGeometry(); // recount sizeHint
    update();         // rerender
}

Qt::Alignment SecureLabel::alignment() const {
    return m_alignment;
}

void SecureLabel::setAlignment(Qt::Alignment align) {
    if (m_alignment != align) {
        m_alignment = align;
        renderTextToPixels();
        update();
    }
}

bool SecureLabel::isObfuscated() const {
    return m_obfuscated;
}

void SecureLabel::setObfuscated(bool obfuscate) {
    if (m_obfuscated != obfuscate) {
        m_obfuscated = obfuscate;
        renderTextToPixels();
        updateGeometry();
        update();
    }
}

QSize SecureLabel::calculateTextBoundingBox() const {
    if (!m_fontLoaded || !m_textData || m_textSize == 0) return QSize(0, 0);

    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize);
    float textWidth = 0;

    const uint8_t* ptr = m_textData;
    const uint8_t* end = ptr + m_textSize;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;

        if (m_obfuscated) {
            cp = 0x2022; // Bullet
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &leftSideBearing);
        textWidth += advanceWidth * scale;
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int textHeight = (ascent - descent) * scale;

    return QSize(static_cast<int>(textWidth + 1), textHeight);
}

QSize SecureLabel::minimumSizeHint() const {
    QSize textSize = calculateTextBoundingBox();
    QMargins margins = contentsMargins();
    int frameWidth = this->frameWidth();

    return QSize(
        textSize.width() + margins.left() + margins.right() + frameWidth * 2,
        textSize.height() + margins.top() + margins.bottom() + frameWidth * 2
        );
}

QSize SecureLabel::sizeHint() const {
    return minimumSizeHint();
}

void SecureLabel::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);

    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        renderTextToPixels();
        update();
    }
}

void SecureLabel::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);

    if (m_imageWidth == contentsRect().width() && m_imageHeight == contentsRect().height()) {
        return;
    }

    renderTextToPixels();
}

void SecureLabel::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    QRect cRect = contentsRect();
    if (m_pixelBuffer.empty() || cRect.isEmpty()) return;

    QPainter painter(this);
    QImage image(
        m_pixelBuffer.data(),
        cRect.width(),
        cRect.height(),
        QImage::Format_ARGB32_Premultiplied
        );

    painter.drawImage(cRect.topLeft(), image);
}

void SecureLabel::renderTextToPixels() {
    QRect cRect = contentsRect();
    qreal dpr = devicePixelRatioF();

    if (!m_fontLoaded || !m_textData || m_textSize == 0 || cRect.isEmpty()) {
        m_pixelBuffer = SecureBuffer(0);
        m_imageWidth = 0;
        m_imageHeight = 0;
        return;
    }

    int imgWidth = qCeil(cRect.width() * dpr);
    int imgHeight = qCeil(cRect.height() * dpr);

    if (m_imageWidth != imgWidth || m_imageHeight != imgHeight || m_pixelBuffer.empty()) {
        m_pixelBuffer = SecureBuffer(imgWidth * imgHeight * 4);
        m_imageWidth = imgWidth;
        m_imageHeight = imgHeight;
    }

    sodium_memzero(m_pixelBuffer.data(), m_pixelBuffer.size());
    uint32_t* destPixels = reinterpret_cast<uint32_t*>(m_pixelBuffer.data());

    // colors and metrics
    QColor textColor = palette().color(QPalette::WindowText);
    uint32_t rColor = textColor.red();
    uint32_t gColor = textColor.green();
    uint32_t bColor = textColor.blue();

    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize * dpr);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int scaledAscent = ascent * scale;

    // alignment
    QSize textSize = calculateTextBoundingBox();
    float cursorX = 0;
    float cursorY = 0;

    if (m_alignment & Qt::AlignHCenter) cursorX = (imgWidth - textSize.width()) / 2.0f;
    else if (m_alignment & Qt::AlignRight) cursorX = imgWidth - textSize.width();

    if (m_alignment & Qt::AlignVCenter) cursorY = (imgHeight - textSize.height()) / 2.0f + scaledAscent;
    else if (m_alignment & Qt::AlignBottom) cursorY = imgHeight - (textSize.height() - scaledAscent);
    else cursorY = scaledAscent;

    // one buffer for each letter
    int maxGlyphSize = static_cast<int>(m_fontSize * 2.0f);
    SecureBuffer reusableGlyphBuffer(maxGlyphSize * maxGlyphSize);

    // render
    const uint8_t* ptr = m_textData;
    const uint8_t* end = ptr + m_textSize;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;
        if (m_obfuscated) cp = 0x2022;

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, cp, scale, scale, &x0, &y0, &x1, &y1);
        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            // owerflow protection
            if (static_cast<size_t>(glyphWidth * glyphHeight) > reusableGlyphBuffer.size()) {
                reusableGlyphBuffer = SecureBuffer(glyphWidth * glyphHeight);
            }

            // clear before next letter render
            sodium_memzero(reusableGlyphBuffer.data(), reusableGlyphBuffer.size());

            // letter render
            stbtt_MakeCodepointBitmap(&m_fontInfo, reusableGlyphBuffer.data(), glyphWidth, glyphHeight, glyphWidth, scale, scale, cp);

            // blending
            for (int y = 0; y < glyphHeight; ++y) {
                for (int x = 0; x < glyphWidth; ++x) {
                    int drawX = static_cast<int>(std::round(cursorX)) + x0 + x;
                    int drawY = static_cast<int>(std::round(cursorY)) + y0 + y;

                    if (drawX >= 0 && drawX < imgWidth && drawY >= 0 && drawY < imgHeight) {
                        uint8_t alpha = reusableGlyphBuffer.data()[y * glyphWidth + x];
                        if (alpha > 0) {
                            uint32_t r = (rColor * alpha) / 255;
                            uint32_t g = (gColor * alpha) / 255;
                            uint32_t b = (bColor * alpha) / 255;
                            destPixels[drawY * imgWidth + drawX] = (alpha << 24) | (r << 16) | (g << 8) | b;
                        }
                    }
                }
            }
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &leftSideBearing);
        cursorX += advanceWidth * scale;
    }

    // ~SecureBuffer
}