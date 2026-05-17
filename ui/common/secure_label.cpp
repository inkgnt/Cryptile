#include "secure_label.h"
#include <cstring>

SecureLabel::SecureLabel(QWidget *parent) : SecureTextWidget(parent) {
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    setObfuscated(false);
}

void SecureLabel::setSecureText(const uint8_t* utf8_data, size_t size) {
    clear();

    if (utf8_data != nullptr && size > 0) {
        m_textCapacity = size + 1;
        m_textBuffer = SecureBuffer(m_textCapacity);
        std::memcpy(m_textBuffer.data(), utf8_data, size);
        m_textLen = size;
    }

    m_needsRender = true;
    updateGeometry();
    update();
}