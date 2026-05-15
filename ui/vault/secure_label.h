#pragma once
#include "utils/secure_buffer.h"

#include <QFrame>
#include <QByteArray>
#include <stb_truetype.h>

class SecureLabel : public QFrame {
    Q_OBJECT

    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool obfuscated READ isObfuscated WRITE setObfuscated)

public:
    explicit SecureLabel(QWidget *parent = nullptr);
    ~SecureLabel() = default;

    void setSecureText(const uint8_t* utf8_data, size_t size);

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment align);

    bool isObfuscated() const;
    void setObfuscated(bool obfuscate);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    void changeEvent(QEvent *event) override;

private:
    void loadFont();
    void renderTextToPixels();
    QRect textRect() const;

    QSize calculateTextBoundingBox() const;

    const uint8_t* m_textData = nullptr;
    size_t m_textSize = 0;

    SecureBuffer m_pixelBuffer{};
    SecureBuffer m_glyphBuffer{};

    size_t m_pixelBufferCapacity = 0;

    int m_imageWidth = 0;
    int m_imageHeight = 0;

    int m_logicalWidth = 0;
    int m_logicalHeight = 0;

    QByteArray m_fontData;
    stbtt_fontinfo m_fontInfo;
    bool m_fontLoaded = false;
    float m_fontSize = 16.0f;

    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;

    bool m_obfuscated = false;
    bool m_needsRender = true;
};