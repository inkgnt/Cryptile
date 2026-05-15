#include "secure_label.h"
#include "utils/widget_helpers.h"

#include <QPainter>
#include <QFile>
#include <QEvent>
#include <QDebug>
#include <QStyleOptionFrame>

SecureLabel::SecureLabel(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    loadFont();
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

QRect SecureLabel::textRect() const {
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.lineWidth = lineWidth();
    opt.midLineWidth = midLineWidth();
    opt.frameShape = frameShape();

    return style()->subElementRect(QStyle::SE_FrameContents, &opt, this);
}

void SecureLabel::setSecureText(const uint8_t* utf8_data, size_t size) {
    m_textData = utf8_data;
    m_textSize = size;
    m_needsRender = true;
    updateGeometry();
    update();
}
Qt::Alignment SecureLabel::alignment() const { return m_alignment; }

void SecureLabel::setAlignment(Qt::Alignment align) {
    if (m_alignment != align) {
        m_alignment = align;
        m_needsRender = true;
        update();
    }
}

bool SecureLabel::isObfuscated() const { return m_obfuscated; }

void SecureLabel::setObfuscated(bool obfuscate) {
    if (m_obfuscated != obfuscate) {
        m_obfuscated = obfuscate;
        m_needsRender = true;
        updateGeometry();
        update();
    }
}

void SecureLabel::changeEvent(QEvent *event) {
    qDebug() << "changeEvent";
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        m_needsRender = true;
        update();
    }
}

void SecureLabel::resizeEvent(QResizeEvent *event) {
    qDebug() << "resizeEvent";
    QFrame::resizeEvent(event);

    QRect cRect = textRect();
    if (m_logicalWidth == cRect.width() && m_logicalHeight == cRect.height()) {
        return;
    }
    m_needsRender = true;
    update();
}
QSize SecureLabel::calculateTextBoundingBox() const {
    if (!m_fontLoaded || !m_textData || m_textSize == 0) return {0, 0};

    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize);
    float textWidth = calculateTextWidthPixels(m_fontInfo, m_textData, m_textSize, scale, m_obfuscated);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int textHeight = std::round((ascent - descent) * scale);

    return {static_cast<int>(textWidth + 1), textHeight};
}

QSize SecureLabel::minimumSizeHint() const {
    QSize textSize = calculateTextBoundingBox();

    QStyleOptionFrame opt;
    opt.initFrom(this);

    opt.rect = QRect(0, 0, 1000, 1000);
    opt.lineWidth = lineWidth();
    opt.midLineWidth = midLineWidth();
    opt.frameShape = frameShape();

    QRect cRect = style()->subElementRect(QStyle::SE_FrameContents, &opt, this);

    int widthPadding = 1000 - cRect.width();
    int heightPadding = 1000 - cRect.height();

    return {
        textSize.width() + widthPadding,
        textSize.height() + heightPadding
    };
}

QSize SecureLabel::sizeHint() const {
    return minimumSizeHint();
}


void SecureLabel::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    QPainter painter(this);

    if (m_needsRender) {
        renderTextToPixels();
        m_needsRender = false;
    }

    QRect cRect = textRect();
    if (m_pixelBuffer.empty() || cRect.isEmpty() || m_imageWidth == 0) return;

    QImage image(
        m_pixelBuffer.data(),
        m_imageWidth,
        m_imageHeight,
        m_imageWidth * 4,
        QImage::Format_ARGB32_Premultiplied
        );

    image.setDevicePixelRatio(devicePixelRatioF());
    painter.drawImage(cRect.topLeft(), image);
}


void SecureLabel::renderTextToPixels() {
    qDebug() << "renderTextToPixels";

    QRect cRect = textRect();
    qreal dpr = devicePixelRatioF();

    if (!m_fontLoaded || !m_textData || m_textSize == 0 || cRect.isEmpty()) {
        m_pixelBuffer =  SecureBuffer();
        m_pixelBufferCapacity = 0;
        m_imageWidth = 0;
        m_imageHeight = 0;
        m_logicalWidth = 0;
        m_logicalHeight = 0;
        return;
    }

    int imgWidth = qCeil(cRect.width() * dpr);
    int imgHeight = qCeil(cRect.height() * dpr);
    size_t requiredSize = imgWidth * imgHeight * 4;

    if (requiredSize > m_pixelBufferCapacity || m_pixelBuffer.empty()) {
        m_pixelBufferCapacity = requiredSize + (requiredSize / 4) + 1024;
        m_pixelBuffer = SecureBuffer(m_pixelBufferCapacity);
    }

    m_imageWidth = imgWidth;
    m_imageHeight = imgHeight;
    m_logicalWidth = cRect.width();
    m_logicalHeight = cRect.height();

    sodium_memzero(m_pixelBuffer.data(), requiredSize);
    uint32_t* destPixels = reinterpret_cast<uint32_t*>(m_pixelBuffer.data());

    QColor textColor = palette().color(foregroundRole());
    uint32_t rColor = textColor.red();
    uint32_t gColor = textColor.green();
    uint32_t bColor = textColor.blue();

    float physScale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize * dpr);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int physAscent = std::round(ascent * physScale);
    int physTextHeight = std::round((ascent - descent) * physScale);

    float cursorX = 0;
    float cursorY = 0;

    if (m_alignment != (Qt::AlignLeft | Qt::AlignVCenter)) {
        float physTextWidth = 0;
        if (m_alignment & (Qt::AlignHCenter | Qt::AlignRight)) {
            physTextWidth = calculateTextWidthPixels(m_fontInfo, m_textData, m_textSize, physScale, m_obfuscated);
        }

        if (m_alignment & Qt::AlignHCenter) cursorX = (imgWidth - physTextWidth) / 2.0f;
        else if (m_alignment & Qt::AlignRight) cursorX = imgWidth - physTextWidth;

        if (m_alignment & Qt::AlignVCenter) cursorY = (imgHeight - physTextHeight) / 2.0f + physAscent;
        else if (m_alignment & Qt::AlignBottom) cursorY = imgHeight - (physTextHeight - physAscent);
        else cursorY = physAscent;
    } else {
        cursorY = (imgHeight - physTextHeight) / 2.0f + physAscent;
    }

    int fontBoxX0, fontBoxY0, fontBoxX1, fontBoxY1;
    stbtt_GetFontBoundingBox(&m_fontInfo, &fontBoxX0, &fontBoxY0, &fontBoxX1, &fontBoxY1);

    int maxGlyphWidth = std::ceil((fontBoxX1 - fontBoxX0) * physScale);
    int maxGlyphHeight = std::ceil((fontBoxY1 - fontBoxY0) * physScale);
    size_t maxGlyphBytes = maxGlyphWidth * maxGlyphHeight;

    if (m_glyphBuffer.size() < maxGlyphBytes) {
        m_glyphBuffer = SecureBuffer(maxGlyphBytes);
    }

    const uint8_t* ptr = m_textData;
    const uint8_t* end = ptr + m_textSize;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;
        if (m_obfuscated) cp = 0x2022;

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, cp, physScale, physScale, &x0, &y0, &x1, &y1);

        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            stbtt_MakeCodepointBitmap(&m_fontInfo, m_glyphBuffer.data(), glyphWidth, glyphHeight, glyphWidth, physScale, physScale, cp);

            int baseDrawX = static_cast<int>(std::round(cursorX)) + x0;
            int baseDrawY = static_cast<int>(std::round(cursorY)) + y0;

            int startX = qMax(0, -baseDrawX);
            int startY = qMax(0, -baseDrawY);
            int endX = qMin(glyphWidth, imgWidth - baseDrawX);
            int endY = qMin(glyphHeight, imgHeight - baseDrawY);

            for (int y = startY; y < endY; ++y) {
                const uint8_t* alphaRow = m_glyphBuffer.data() + (y * glyphWidth);
                uint32_t* destRow = destPixels + ((baseDrawY + y) * imgWidth);

                for (int x = startX; x < endX; ++x) {
                    uint8_t alpha = alphaRow[x];
                    if (alpha > 0) {
                        uint32_t r = (rColor * alpha) >> 8;
                        uint32_t g = (gColor * alpha) >> 8;
                        uint32_t b = (bColor * alpha) >> 8;

                        destRow[baseDrawX + x] = (alpha << 24) | (r << 16) | (g << 8) | b;
                    }
                }
            }
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &leftSideBearing);
        cursorX += std::round(advanceWidth * physScale);
    }
}