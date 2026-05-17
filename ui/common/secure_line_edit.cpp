#include "secure_line_edit.h"
#include "utils/widget_helpers.h"
#include <QStyleOptionFrame>
#include <sodium.h>

SecureLineEdit::SecureLineEdit(QWidget *parent) : SecureTextWidget(parent) {
    setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    //setAttribute(Qt::WA_TranslucentBackground);

    setFocusPolicy(Qt::StrongFocus);

    setCursor(Qt::IBeamCursor);
}
QSize SecureLineEdit::minimumSizeHint() const {
    QSize baseSize = SecureTextWidget::minimumSizeHint();

    return QSize(50 + contentsMargins().left() + contentsMargins().right(), baseSize.height());
}

QSize SecureLineEdit::sizeHint() const {
    QSize baseSize = SecureTextWidget::minimumSizeHint();

    SecureBuffer xChar(1);
    xChar.data()[0] = 'x';
    int charWidth = std::round(m_renderer.calculateTextWidth(xChar, 1));
    if (charWidth == 0) charWidth = 10;

    int defaultWidth = 15 * charWidth;

    QStyleOptionFrame opt;
    initStyleOptionForText(&opt);
    opt.rect = QRect(0, 0, 1000, 1000);
    QRect cRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);

    return QSize(defaultWidth + (1000 - cRect.width()) + 2, baseSize.height());
}

void SecureLineEdit::ensureCursorVisible() {
    size_t renderLen = 0;
    SecureBuffer buf = getRenderText(renderLen);
    if (renderLen == 0) {
        m_scrollOffset = 0.0f;
        return;
    }

    float cursorLogicalX = m_renderer.charIndexToOffset(m_cursorCharIdx, buf, renderLen);
    float textWidth = m_renderer.calculateTextWidth(buf, renderLen);
    QRect cRect = textRect();

    const float cursorMargin = 4.0f;

    if (cursorLogicalX - m_scrollOffset < cursorMargin) {
        m_scrollOffset = std::max(0.0f, cursorLogicalX - cursorMargin);
    }
    else if (cursorLogicalX - m_scrollOffset > cRect.width() - cursorMargin) {
        m_scrollOffset = cursorLogicalX - cRect.width() + cursorMargin;
    }

    if (textWidth <= cRect.width()) {
        m_scrollOffset = 0.0f;
    } else if (m_scrollOffset > textWidth - cRect.width() + cursorMargin) {
        m_scrollOffset = textWidth - cRect.width() + cursorMargin;
    }

    m_needsRender = true;
}

void SecureLineEdit::resizeEvent(QResizeEvent *event) {
    SecureTextWidget::resizeEvent(event);
    ensureCursorVisible();
    update();
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
    ensureCursorVisible();
    updateGeometry();
    update();
}

void SecureLineEdit::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {

        size_t renderLen = 0;
        SecureBuffer buf = getRenderText(renderLen);

        float localX = event->pos().x() - (textRect().left() + m_textStartX);
        m_cursorCharIdx = m_renderer.offsetToCharIndex(localX, buf, renderLen);

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            m_selectionStartCharIdx = m_cursorCharIdx;
        }

        if (m_cursorTimerId != 0) killTimer(m_cursorTimerId);
        m_cursorTimerId = startTimer(500);
        m_cursorVisible = true;
        m_needsRender = true;
        ensureCursorVisible();
        update();
    }
}


// TODO SCROLL ACCELERATION
void SecureLineEdit::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        QRect cRect = textRect();
        int x = event->pos().x();

        if (x < cRect.left()) {
            m_autoScrollDirection = -1;
            if (m_autoScrollTimerId == 0) m_autoScrollTimerId = startTimer(5);
        }

        else if (x > cRect.right()) {
            m_autoScrollDirection = 1;
            if (m_autoScrollTimerId == 0) m_autoScrollTimerId = startTimer(5);
        }

        else {
            if (m_autoScrollTimerId != 0) {
                killTimer(m_autoScrollTimerId);
                m_autoScrollTimerId = 0;
            }

            size_t renderLen = 0;
            SecureBuffer buf = getRenderText(renderLen);
            float localX = x - (cRect.left() + m_textStartX);
            m_cursorCharIdx = m_renderer.offsetToCharIndex(localX, buf, renderLen);

            m_needsRender = true;
            ensureCursorVisible();
            update();
        }
    }
}

void SecureLineEdit::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && m_autoScrollTimerId != 0) {
        killTimer(m_autoScrollTimerId);
        m_autoScrollTimerId = 0;
    }
    SecureTextWidget::mouseReleaseEvent(event);
}




void SecureLineEdit::keyPressEvent(QKeyEvent *event) {
    if (event->matches(QKeySequence::SelectAll)) {
        m_selectionStartCharIdx = 0;
        m_cursorCharIdx = totalChars();
        m_needsRender = true;
        ensureCursorVisible();
        update();
        return;
    }

    if (event->key() == Qt::Key_Backspace) {
        if (!deleteSelectedText() && m_cursorCharIdx > 0) {
            m_selectionStartCharIdx = (event->modifiers() & Qt::ControlModifier) ? 0 : m_cursorCharIdx - 1;
            deleteSelectedText();
        }
        m_needsRender = true; ensureCursorVisible(); updateGeometry(); update();
    }
    else if (event->key() == Qt::Key_Delete) {
        if (!deleteSelectedText() && m_cursorCharIdx < totalChars()) {
            m_selectionStartCharIdx = (event->modifiers() & Qt::ControlModifier) ? totalChars() : m_cursorCharIdx + 1;
            deleteSelectedText();
        }
        m_needsRender = true; ensureCursorVisible(); updateGeometry(); update();
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
        m_needsRender = true; ensureCursorVisible(); update();
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
        m_needsRender = true; ensureCursorVisible(); update();
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

void SecureLineEdit::focusInEvent(QFocusEvent *event) {
    SecureTextWidget::focusInEvent(event);
    m_cursorTimerId = startTimer(500);
    m_cursorVisible = true;
    ensureCursorVisible();
    update();
}

void SecureLineEdit::focusOutEvent(QFocusEvent *event) {
    SecureTextWidget::focusOutEvent(event);
    if (m_cursorTimerId != 0) {
        killTimer(m_cursorTimerId);
        m_cursorTimerId = 0;
    }
    m_cursorVisible = false;

    if (m_selectionStartCharIdx != m_cursorCharIdx) {
        m_selectionStartCharIdx = m_cursorCharIdx;
        m_needsRender = true;
    }
    ensureCursorVisible();
    update();
}

void SecureLineEdit::timerEvent(QTimerEvent *event) {
    if (event->timerId() == m_cursorTimerId) {
        m_cursorVisible = !m_cursorVisible;
        update();
    }

    else if (event->timerId() == m_autoScrollTimerId) {
        int oldIdx = m_cursorCharIdx;

        if (m_autoScrollDirection < 0) {
            m_cursorCharIdx = qMax(0, m_cursorCharIdx - 1);
        } else {
            m_cursorCharIdx = qMin(totalChars(), m_cursorCharIdx + 1);
        }

        if (oldIdx != m_cursorCharIdx) {
            ensureCursorVisible();
            m_needsRender = true;
            update();
        }
    }
    else {
        SecureTextWidget::timerEvent(event);
    }
}