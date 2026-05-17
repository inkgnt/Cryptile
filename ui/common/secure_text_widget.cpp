#include "secure_text_widget.h"
#include "utils/widget_helpers.h"
#include <QStyleOptionFrame>
#include <QPainter>
#include <QEvent>
#include <sodium.h>

SecureTextWidget::SecureTextWidget(QWidget *parent) : QFrame(parent) {
    setAttribute(Qt::WA_StyledBackground, true);
    setFrameStyle(QFrame::NoFrame | QFrame::Plain);

    m_renderer.loadFont(m_fontPath);
    m_renderer.setFontSize(m_fontSize);
}

void SecureTextWidget::clear() {
    if (m_textCapacity > 0)
        sodium_memzero(m_textBuffer.data(), m_textCapacity);

    m_textLen = 0;
    m_cursorCharIdx = 0;
    m_selectionStartCharIdx = 0;
    m_needsRender = true;
    updateGeometry();
    update();
}

void SecureTextWidget::initStyleOptionForText(QStyleOptionFrame *opt) const {
    opt->initFrom(this);

    if (frameShape() != QFrame::NoFrame) {
        opt->lineWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth, opt, this);
        opt->state |= QStyle::State_Sunken;
    } else {
        opt->lineWidth = 0;
        opt->features |= QStyleOptionFrame::Flat;
    }
}


SecureBuffer SecureTextWidget::getSecureText() const {
    if (m_textLen == 0)
        return SecureBuffer();

    SecureBuffer copy(m_textLen);
    std::memcpy(copy.data(), m_textBuffer.data(), m_textLen);
    return copy;
}

void SecureTextWidget::setPlaceholderText(const QString& placeholder) {
    if (m_placeholderText != placeholder) { m_placeholderText = placeholder; update(); }
}
void SecureTextWidget::setAlignment(Qt::Alignment align) {
    m_alignment = align; m_needsRender = true; update();
}
void SecureTextWidget::setObfuscated(bool obfuscate) {
    m_obfuscated = obfuscate; m_needsRender = true; updateGeometry(); update();
}

QRect SecureTextWidget::textRect() const {
    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);

    if (frameShape() != QFrame::NoFrame) {
        QRect r = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
        return r.marginsRemoved(contentsMargins());
    } else {
        return rect().marginsRemoved(contentsMargins());
    }
}
int SecureTextWidget::totalChars() const {
    int count = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while(ptr < end) { nextUtf8Codepoint(ptr, end); count++; }
    return count;
}

size_t SecureTextWidget::charIndexToByteOffset(int targetIdx) const {
    int idx = 0;
    const uint8_t* ptr = m_textBuffer.data();
    const uint8_t* end = ptr + m_textLen;
    while (ptr < end && idx < targetIdx) { nextUtf8Codepoint(ptr, end); idx++; }
    return ptr - m_textBuffer.data();
}

SecureBuffer SecureTextWidget::getRenderText(size_t& outLen) const {
    if (!m_obfuscated) {
        outLen = m_textLen;
        SecureBuffer copy(m_textLen);
        std::memcpy(copy.data(), m_textBuffer.data(), m_textLen);
        return copy;
    }

    int count = totalChars();
    outLen = count * 3;
    SecureBuffer dots(outLen);
    for (int i = 0; i < count; ++i) {
        dots.data()[i * 3 + 0] = 0xE2;
        dots.data()[i * 3 + 1] = 0x80;
        dots.data()[i * 3 + 2] = 0xA2;
    }
    return dots;
}

QSize SecureTextWidget::minimumSizeHint() const {
    if (!m_renderer.isLoaded())
        return {0, 0};

    float textWidth = 0.0f;
    if (m_textLen > 0) {
        size_t renderLen = 0;
        SecureBuffer buf = getRenderText(renderLen);
        textWidth = m_renderer.calculateTextWidth(buf, renderLen);
    } else if (!m_placeholderText.isEmpty()) {
        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        QFontMetrics fm(f);
        textWidth = fm.horizontalAdvance(m_placeholderText);
    }

    auto metrics = m_renderer.getMetrics();
    int textHeight = std::round(metrics.textHeight);

    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    opt.rect = QRect(0, 0, 1000, 1000);

    if (frameShape() != QFrame::NoFrame) {
        QRect cRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
        return {
            static_cast<int>(textWidth) + (1000 - cRect.width()) + 2,
            textHeight + (1000 - cRect.height())
        };
    } else {
        return {
            static_cast<int>(textWidth) + contentsMargins().left() + contentsMargins().right() + 2,
            textHeight + contentsMargins().top() + contentsMargins().bottom()
        };
    }
}
QSize SecureTextWidget::sizeHint() const { return minimumSizeHint(); }

void SecureTextWidget::paintEvent(QPaintEvent *event) {
    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    QPainter painter(this);

    if (frameShape() != QFrame::NoFrame) {
        style()->drawPrimitive(QStyle::PE_PanelLineEdit, &opt, &painter, this);
    } else {
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    }

    QRect cRect = textRect();
    if (cRect.isEmpty())
        return;

    if (m_textLen == 0 && !m_placeholderText.isEmpty()) {
        painter.setPen(palette().color(QPalette::PlaceholderText));
        QFont f = font();
        f.setPixelSize(static_cast<int>(m_fontSize));
        painter.setFont(f);
        painter.drawText(cRect, m_alignment, m_placeholderText);

        if (hasFocus() && m_cursorVisible) {
            painter.setPen(palette().color(foregroundRole()));
            int cx = cRect.left();
            if (m_alignment & Qt::AlignHCenter) cx += cRect.width() / 2;
            painter.drawLine(cx, cRect.top(), cx, cRect.bottom());
        }
        return;
    }

    size_t renderLen = 0;
    SecureBuffer renderBuf = getRenderText(renderLen);

    int selMin = qMin(m_selectionStartCharIdx, m_cursorCharIdx);
    int selMax = qMax(m_selectionStartCharIdx, m_cursorCharIdx);

    if (m_needsRender) {
        m_renderer.renderText(
            renderBuf, renderLen, cRect, devicePixelRatioF(), m_alignment,
            m_scrollOffset,
            selMin, selMax, palette().color(foregroundRole()), palette().color(QPalette::HighlightedText),
            m_textStartX, m_textStartY
            );
        m_needsRender = false;
    }

    float globalStartX = cRect.left() + m_textStartX;
    float globalStartY = cRect.top() + m_textStartY;
    auto metrics = m_renderer.getMetrics();

    if (selMin != selMax && renderLen > 0) {
        float x1 = m_renderer.charIndexToOffset(selMin, renderBuf, renderLen);
        float x2 = m_renderer.charIndexToOffset(selMax, renderBuf, renderLen);
        QRectF highlightRect(globalStartX + x1, globalStartY, x2 - x1, metrics.textHeight);
        painter.fillRect(highlightRect, palette().color(QPalette::Highlight));
    }

    if (!m_renderer.pixelBuffer().empty() && m_renderer.imageWidth() > 0) {
        QImage image(m_renderer.pixelBuffer().data(), m_renderer.imageWidth(), m_renderer.imageHeight(),
                     m_renderer.imageWidth() * 4, QImage::Format_ARGB32_Premultiplied);
        image.setDevicePixelRatio(devicePixelRatioF());
        painter.drawImage(cRect.topLeft(), image);
    }

    if (hasFocus() && m_cursorVisible) {
        painter.setPen(palette().color(foregroundRole()));
        float cursorOffset = m_renderer.charIndexToOffset(m_cursorCharIdx, renderBuf, renderLen);
        float cx = globalStartX + cursorOffset;
        painter.drawLine(QPointF(cx, globalStartY), QPointF(cx, globalStartY + metrics.textHeight));
    }
}

void SecureTextWidget::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);
    m_needsRender = true;
    update();
}

void SecureTextWidget::changeEvent(QEvent *event) {
    QFrame::changeEvent(event);

    if (event->type() == QEvent::PaletteChange || event->type() == QEvent::StyleChange ||
        event->type() == QEvent::FontChange
        )
    {

        m_renderer.setFontSize(m_fontSize);

        m_needsRender = true;
        updateGeometry();
        update();
    }
}