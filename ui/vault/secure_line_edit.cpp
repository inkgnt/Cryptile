#include "secure_line_edit.h"
#include "utils/widget_helpers.h"

#include <QPainter>
#include <QFile>
#include <QEvent>
#include <QKeyEvent>
#include <QTimerEvent>
#include <QDebug>
#include <QStyleOption>
#include <sodium.h>
#include <cmath>
//todo  нужно добавить поддержку добавление кнопок и иконок через QToolButton и QAction
//todo  make sure that QSS is working properly
//todo  нужно разобраться с отступами для текста*/

SecureLineEdit::SecureLineEdit(QWidget *parent) : QFrame(parent) {
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);
    setAttribute(Qt::WA_TranslucentBackground);

    setFocusPolicy(Qt::StrongFocus);

    setCursor(Qt::IBeamCursor);

    loadFont();
}

void SecureLineEdit::loadFont() {
    QFile fontFile(":/fonts/RobotoMono-Regular");
    if (!fontFile.open(QIODevice::ReadOnly)) return;
    m_fontData = fontFile.readAll();
    const unsigned char* fontPtr = reinterpret_cast<const unsigned char*>(m_fontData.constData());
    if (stbtt_InitFont(&m_fontInfo, fontPtr, stbtt_GetFontOffsetForIndex(fontPtr, 0))) {
        m_fontLoaded = true;
    }
}

QString SecureLineEdit::placeholderText() const {
    return m_placeholderText;
}

void SecureLineEdit::setPlaceholderText(const QString& placeholder) {
    if (m_placeholderText != placeholder) {
        m_placeholderText = placeholder;
        update();
    }
}

Qt::Alignment SecureLineEdit::alignment() const { return m_alignment; }

void SecureLineEdit::setAlignment(Qt::Alignment align) { m_alignment = align; m_needsRender = true; update(); }

bool SecureLineEdit::isObfuscated() const { return m_obfuscated; }

void SecureLineEdit::setObfuscated(bool obfuscate) { m_obfuscated = obfuscate; m_needsRender = true; updateGeometry(); update(); }

int SecureLineEdit::totalChars() const {
    int count = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while(ptr < end) { nextUtf8Codepoint(ptr, end); count++; }
    return count;
}

size_t SecureLineEdit::charIndexToByteOffset(int targetIdx) const {
    int idx = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while (ptr < end && idx < targetIdx) {
        nextUtf8Codepoint(ptr, end);
        idx++;
    }
    return ptr - m_textBuffer.data();
}

SecureBuffer SecureLineEdit::getSecureText() const {
    if (m_textLen == 0) return SecureBuffer(0);
    SecureBuffer copy(m_textLen);
    std::memcpy(copy.data(), m_textBuffer.data(), m_textLen);
    return copy;
}


void SecureLineEdit::clear() {
    if (m_textCapacity > 0) sodium_memzero(m_textBuffer.data(), m_textCapacity);
    m_textLen = 0;
    m_cursorCharIdx = 0;
    m_selectionStartCharIdx = 0;
    m_needsRender = true;
    updateGeometry();
    update();
}

void SecureLineEdit::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        m_cursorCharIdx = xToCharIndex(event->pos().x());
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            m_selectionStartCharIdx = m_cursorCharIdx;
        }
        if (m_cursorTimerId != 0) killTimer(m_cursorTimerId);
        m_cursorTimerId = startTimer(500);
        m_cursorVisible = true;
        m_needsRender = true;
        update();
    }
}

void SecureLineEdit::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        m_cursorCharIdx = xToCharIndex(event->pos().x());
        m_needsRender = true;
        update();
    }
}

void SecureLineEdit::focusInEvent(QFocusEvent *event) {
    QFrame::focusInEvent(event);
    m_cursorTimerId = startTimer(500);
    m_cursorVisible = true;
    update();
}

void SecureLineEdit::focusOutEvent(QFocusEvent *event) {
    QFrame::focusOutEvent(event);
    if (m_cursorTimerId != 0) {
        killTimer(m_cursorTimerId);
        m_cursorTimerId = 0;
    }
    m_cursorVisible = false;

    if (m_selectionStartCharIdx != m_cursorCharIdx) {
        m_selectionStartCharIdx = m_cursorCharIdx;
        m_needsRender = true;
    }
    update();
}

void SecureLineEdit::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_cursorTimerId) {
        m_cursorVisible = !m_cursorVisible;
        update();
    } else {
        QFrame::timerEvent(event);
    }
}

void SecureLineEdit::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);
    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange) {
        m_needsRender = true;
        update();
    }
}

void SecureLineEdit::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    QRect cRect = textRect();
    if (m_logicalWidth == cRect.width() && m_logicalHeight == cRect.height()) return;
    m_needsRender = true;
    update();
}

void SecureLineEdit::clearCanvas() {
    m_pixelBuffer = SecureBuffer(0);
    m_pixelBufferCapacity = 0;
    m_imageWidth = 0; m_imageHeight = 0;
    m_logicalWidth = 0; m_logicalHeight = 0;
}


QSize SecureLineEdit::minimumSizeHint() const {
    if (!m_fontLoaded) return {0, 0};
    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize);

    float textWidth = 0.0f;
    if (m_textLen > 0) {
        textWidth = calculateTextWidthPixels(m_fontInfo, m_textBuffer.data(), m_textLen, scale, m_obfuscated);
    } else if (!m_placeholderText.isEmpty()) {
        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        QFontMetrics fm(f);
        textWidth = fm.horizontalAdvance(m_placeholderText);
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int textHeight = std::round((ascent - descent) * scale);

    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.rect = QRect(0, 0, 1000, 1000);
    opt.lineWidth = lineWidth();
    opt.midLineWidth = midLineWidth();
    opt.frameShape = frameShape();
    QRect cRect = style()->subElementRect(QStyle::SE_FrameContents, &opt, this);

    return {
        static_cast<int>(textWidth) + (1000 - cRect.width()) + 2,
        textHeight + (1000 - cRect.height())
    };
}

QSize SecureLineEdit::sizeHint() const { return minimumSizeHint(); }

QRect SecureLineEdit::textRect() const {
    QStyleOptionFrame opt;
    opt.initFrom(this);
    opt.lineWidth = lineWidth();
    opt.midLineWidth = midLineWidth();
    opt.frameShape = frameShape();
    return style()->subElementRect(QStyle::SE_FrameContents, &opt, this);
}



int SecureLineEdit::xToCharIndex(int targetX) const {
    if (m_textLen == 0 || !m_fontLoaded) return 0;
    QRect cRect = textRect();
    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize);
    float textWidth = calculateTextWidthPixels(m_fontInfo, m_textBuffer.data(), m_textLen, scale, m_obfuscated);

    float cursorX = 0;
    if (m_alignment & Qt::AlignHCenter) cursorX = (cRect.width() - textWidth) / 2.0f;
    else if (m_alignment & Qt::AlignRight) cursorX = cRect.width() - textWidth;

    float currentX = cRect.left() + cursorX;
    int charIdx = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;

    while (ptr < end) {
        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (m_obfuscated) cp = 0x2022;
        int advanceWidth, lsb;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &lsb);
        float step = advanceWidth * scale;
        if (targetX < currentX + step / 2.0f) return charIdx;
        currentX += step;
        charIdx++;
    }
    return charIdx;
}

bool SecureLineEdit::deleteSelectedText() {
    if (m_selectionStartCharIdx == m_cursorCharIdx) return false;

    int minIdx = qMin(m_selectionStartCharIdx, m_cursorCharIdx);
    int maxIdx = qMax(m_selectionStartCharIdx, m_cursorCharIdx);

    size_t offsetStart = charIndexToByteOffset(minIdx);
    size_t offsetEnd = charIndexToByteOffset(maxIdx);
    size_t bytesToDelete = offsetEnd - offsetStart;

    if (m_textLen > offsetEnd) {
        std::memmove(m_textBuffer.data() + offsetStart, m_textBuffer.data() + offsetEnd, m_textLen - offsetEnd);
    }
    sodium_memzero(m_textBuffer.data() + m_textLen - bytesToDelete, bytesToDelete);
    m_textLen -= bytesToDelete;

    m_cursorCharIdx = minIdx;
    m_selectionStartCharIdx = minIdx;
    return true;
}


void SecureLineEdit::insertText(const SecureBuffer& utf8) {
    if (utf8.empty()) return;
    deleteSelectedText();

    size_t newLen = m_textLen + utf8.size();
    size_t offset = charIndexToByteOffset(m_cursorCharIdx);

    if (newLen > m_textCapacity || m_textBuffer.empty()) {
        size_t newCapacity = newLen + 32;
        SecureBuffer newBuffer(newCapacity);
        sodium_memzero(newBuffer.data(), newCapacity);

        if (offset > 0) std::memcpy(newBuffer.data(), m_textBuffer.data(), offset);
        std::memcpy(newBuffer.data() + offset, utf8.data(), utf8.size());
        if (m_textLen > offset) std::memcpy(newBuffer.data() + offset + utf8.size(), m_textBuffer.data() + offset, m_textLen - offset);

        m_textBuffer = std::move(newBuffer);
        m_textCapacity = newCapacity;
    } else {
        if (m_textLen > offset) std::memmove(m_textBuffer.data() + offset + utf8.size(), m_textBuffer.data() + offset, m_textLen - offset);
        std::memcpy(m_textBuffer.data() + offset, utf8.data(), utf8.size());
    }
    m_textLen = newLen;

    int insertedChars = 0;
    const uint8_t* p = utf8.data();
    const uint8_t* e = p + utf8.size();
    while (p < e) { nextUtf8Codepoint(p, e); insertedChars++; }

    m_cursorCharIdx += insertedChars;
    m_selectionStartCharIdx = m_cursorCharIdx;

    m_needsRender = true;
    updateGeometry();
    update();
}

void SecureLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::SelectAll)) {
        m_selectionStartCharIdx = 0;
        m_cursorCharIdx = totalChars();
        m_needsRender = true;
        update();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        if (!deleteSelectedText() && m_cursorCharIdx > 0) {
            m_selectionStartCharIdx = m_cursorCharIdx - 1;
            deleteSelectedText();
        }
        m_needsRender = true; updateGeometry(); update();
    }
    else if (event->key() == Qt::Key_Delete) {
        if (!deleteSelectedText() && m_cursorCharIdx < totalChars()) {
            m_selectionStartCharIdx = m_cursorCharIdx + 1;
            deleteSelectedText();
        }
        m_needsRender = true; updateGeometry(); update();
    }
    else if (event->key() == Qt::Key_Left) {
        int target = qMax(0, m_cursorCharIdx - 1);
        if (event->modifiers() & Qt::ShiftModifier) {
            m_cursorCharIdx = target;
        } else {
            if (m_selectionStartCharIdx != m_cursorCharIdx) m_cursorCharIdx = qMin(m_selectionStartCharIdx, m_cursorCharIdx);
            else m_cursorCharIdx = target;
            m_selectionStartCharIdx = m_cursorCharIdx;
        }
        m_needsRender = true; update();
    }
    else if (event->key() == Qt::Key_Right) {
        int target = qMin(totalChars(), m_cursorCharIdx + 1);
        if (event->modifiers() & Qt::ShiftModifier) {
            m_cursorCharIdx = target;
        } else {
            if (m_selectionStartCharIdx != m_cursorCharIdx) m_cursorCharIdx = qMax(m_selectionStartCharIdx, m_cursorCharIdx);
            else m_cursorCharIdx = target;
            m_selectionStartCharIdx = m_cursorCharIdx;
        }
        m_needsRender = true; update();
    }
    else if (!event->text().isEmpty()) {
        const QString& t = event->text();
        if (t.length() > 0 && t[0].isPrint()) {
            QByteArray tempUtf8 = t.toUtf8();
            SecureBuffer utf8(tempUtf8.size());
            std::memcpy(utf8.data(), tempUtf8.constData(), tempUtf8.size());
            insertText(utf8);
            sodium_memzero(const_cast<char*>(tempUtf8.data()), tempUtf8.capacity());
        }
    }

    if (m_cursorTimerId != 0) {
        killTimer(m_cursorTimerId);
        m_cursorTimerId = startTimer(500);
        m_cursorVisible = true;
    }
}

void SecureLineEdit::paintEvent(QPaintEvent *event) {
    QFrame::paintEvent(event);

    if (m_needsRender) {
        renderTextToPixels();
        m_needsRender = false;
    }

    QRect cRect = textRect();
    QPainter painter(this);


    if (m_textLen == 0 && !m_placeholderText.isEmpty()) {
        QColor phColor = palette().color(QPalette::PlaceholderText);

        if(!phColor.isValid()) {
            phColor = palette().color(foregroundRole());
            phColor.setAlpha(128);
        }

        painter.setPen(phColor);

        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        painter.setFont(f);

        painter.drawText(cRect, m_alignment, m_placeholderText);
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize);
    int textHeight = std::round((ascent - descent) * scale);

    bool hasSelection = (m_selectionStartCharIdx != m_cursorCharIdx);
    if (hasSelection && m_textLen > 0 && !m_logicalTextRect.isEmpty()) {
        int x = cRect.left() + m_logicalSelStartX;
        int w = m_logicalSelEndX - m_logicalSelStartX;

        QRect highlightRect(x, cRect.top() + m_logicalCursorY, w, textHeight);
        painter.fillRect(highlightRect, palette().color(QPalette::Highlight));
    }

    if (!m_pixelBuffer.empty() && !cRect.isEmpty() && m_imageWidth > 0) {
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

    if (hasFocus() && m_cursorVisible) {
        painter.setPen(palette().color(foregroundRole()));

        int cx = cRect.left() + m_logicalCursorX;
        int cy = cRect.top() + m_logicalCursorY;

        painter.drawLine(cx, cy, cx, cy + textHeight);
    }
}

void SecureLineEdit::renderTextToPixels() {
    QRect cRect = textRect();
    qreal dpr = devicePixelRatioF();

    if (!m_fontLoaded || cRect.isEmpty()) {
        clearCanvas();
        return;
    }

    int imgWidth = qCeil(cRect.width() * dpr);
    int imgHeight = qCeil(cRect.height() * dpr);

    float physScale = stbtt_ScaleForPixelHeight(&m_fontInfo, m_fontSize * dpr);

    float physTextWidth = 0.0f;
    if (m_textLen > 0) {
        physTextWidth = calculateTextWidthPixels(m_fontInfo, m_textBuffer.data(), m_textLen, physScale, m_obfuscated);
    }

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int physAscent = std::round(ascent * physScale);
    int physTextHeight = std::round((ascent - descent) * physScale);

    float cursorX = 0;
    float cursorY = 0;

    if (m_alignment != (Qt::AlignLeft | Qt::AlignVCenter)) {
        if (m_alignment & Qt::AlignHCenter) cursorX = (imgWidth - physTextWidth) / 2.0f;
        else if (m_alignment & Qt::AlignRight) cursorX = imgWidth - physTextWidth;

        if (m_alignment & Qt::AlignVCenter) cursorY = (imgHeight - physTextHeight) / 2.0f + physAscent;
        else if (m_alignment & Qt::AlignBottom) cursorY = imgHeight - (physTextHeight - physAscent);
        else cursorY = physAscent;
    } else {
        cursorY = (imgHeight - physTextHeight) / 2.0f + physAscent;
    }

    float physSelStartX = cursorX;
    float physSelEndX = cursorX;
    float physCursorX = cursorX;
    m_logicalCursorY = static_cast<int>((cursorY - physAscent) / dpr);

    if (m_textLen == 0) {
        m_logicalCursorX = static_cast<int>(std::round(cursorX / dpr));
        m_logicalSelStartX = m_logicalCursorX;
        m_logicalSelEndX = m_logicalCursorX;
        m_logicalTextRect = QRect(m_logicalCursorX, m_logicalCursorY, 0, std::round(physTextHeight / dpr));
        clearCanvas();
        return;
    }

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

    int selMin = qMin(m_selectionStartCharIdx, m_cursorCharIdx);
    int selMax = qMax(m_selectionStartCharIdx, m_cursorCharIdx);
    bool hasSelection = (selMin != selMax);

    QColor normalColor = palette().color(foregroundRole());
    QColor highlightColor = palette().color(QPalette::HighlightedText);

    uint32_t nR = normalColor.red(), nG = normalColor.green(), nB = normalColor.blue();
    uint32_t hR = highlightColor.red(), hG = highlightColor.green(), hB = highlightColor.blue();

    int fontBoxX0, fontBoxY0, fontBoxX1, fontBoxY1;
    stbtt_GetFontBoundingBox(&m_fontInfo, &fontBoxX0, &fontBoxY0, &fontBoxX1, &fontBoxY1);
    int maxGlyphWidth = std::ceil((fontBoxX1 - fontBoxX0) * physScale);
    int maxGlyphHeight = std::ceil((fontBoxY1 - fontBoxY0) * physScale);
    size_t maxGlyphBytes = maxGlyphWidth * maxGlyphHeight;

    if (m_glyphBuffer.size() < maxGlyphBytes) {
        m_glyphBuffer = SecureBuffer(maxGlyphBytes);
    }

    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    int charIdx = 0;

    while (ptr < end) {
        if (charIdx == selMin) physSelStartX = cursorX;
        if (charIdx == selMax) physSelEndX = cursorX;
        if (charIdx == m_cursorCharIdx) physCursorX = cursorX;

        uint32_t cp = nextUtf8Codepoint(ptr, end);
        if (cp == 0) break;
        if (m_obfuscated) cp = 0x2022;

        bool isSelected = hasSelection && (charIdx >= selMin && charIdx < selMax);
        uint32_t rColor = isSelected ? hR : nR;
        uint32_t gColor = isSelected ? hG : nG;
        uint32_t bColor = isSelected ? hB : nB;

        int x0, y0, x1, y1;
        stbtt_GetCodepointBitmapBox(&m_fontInfo, cp, physScale, physScale, &x0, &y0, &x1, &y1);
        int glyphWidth = x1 - x0;
        int glyphHeight = y1 - y0;

        if (glyphWidth > 0 && glyphHeight > 0) {
            if (static_cast<size_t>(glyphWidth * glyphHeight) <= m_glyphBuffer.size()) {
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
                            uint32_t bg = destRow[baseDrawX + x];
                            if (bg == 0) {
                                uint32_t r = (rColor * alpha) / 255;
                                uint32_t g = (gColor * alpha) / 255;
                                uint32_t b = (bColor * alpha) / 255;
                                destRow[baseDrawX + x] = (alpha << 24) | (r << 16) | (g << 8) | b;
                            } else {
                                uint32_t bgA = (bg >> 24) & 0xFF;
                                uint32_t bgR = (bg >> 16) & 0xFF;
                                uint32_t bgG = (bg >> 8) & 0xFF;
                                uint32_t bgB = bg & 0xFF;

                                uint32_t invAlpha = 255 - alpha;
                                uint32_t r = ((rColor * alpha) + bgR * invAlpha) / 255;
                                uint32_t g = ((gColor * alpha) + bgG * invAlpha) / 255;
                                uint32_t b = ((bColor * alpha) + bgB * invAlpha) / 255;
                                uint32_t a = alpha + (bgA * invAlpha) / 255;

                                destRow[baseDrawX + x] = (a << 24) | (r << 16) | (g << 8) | b;
                            }
                        }
                    }
                }
            }
        }

        int advanceWidth, leftSideBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, cp, &advanceWidth, &leftSideBearing);
        cursorX += advanceWidth * physScale;
        charIdx++;
    }

    if (charIdx == selMin) physSelStartX = cursorX;
    if (charIdx == selMax) physSelEndX = cursorX;
    if (charIdx == m_cursorCharIdx) physCursorX = cursorX;

    m_logicalCursorX = static_cast<int>(std::round(physCursorX / dpr));
    m_logicalSelStartX = static_cast<int>(std::round(physSelStartX / dpr));
    m_logicalSelEndX = static_cast<int>(std::round(physSelEndX / dpr));

    m_logicalTextRect = QRect(
        m_logicalSelStartX,
        m_logicalCursorY,
        m_logicalSelEndX - m_logicalSelStartX,
        std::round(physTextHeight / dpr)
    );
}