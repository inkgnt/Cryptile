#pragma once
#include "secure_text_widget.h"

class SecureLabel : public SecureTextWidget {
    Q_OBJECT
public:
    explicit SecureLabel(QWidget *parent = nullptr);
    ~SecureLabel() override = default;

    void setSecureText(const uint8_t* utf8_data, size_t size);
};