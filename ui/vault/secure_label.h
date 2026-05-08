#pragma once

#include <QFrame>
#include <QByteArray>
#include "utils/secure_buffer.h"
#include <stb_truetype.h>

class SecureLabel : public QFrame {
    Q_OBJECT

    // Qt-свойства для работы с Qt Designer и QSS
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool obfuscated READ isObfuscated WRITE setObfuscated)

public:
    explicit SecureLabel(QWidget *parent = nullptr);
    ~SecureLabel() override;

    // Главный метод установки данных
    void setSecureText(const uint8_t* utf8_data, size_t size);

    // Выравнивание
    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment align);

    // Режим скрытия (точки вместо текста)
    bool isObfuscated() const;
    void setObfuscated(bool obfuscate);

    // Size Hints для Layout-ов
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

    // Ловим смену системной палитры (темная/светлая тема ОС)
    void changeEvent(QEvent *event) override;

private:
    void loadFont();
    void renderTextToPixels();

    // Метод для расчета размера текста БЕЗ отрисовки
    QSize calculateTextBoundingBox() const;

    const uint8_t* m_textData = nullptr;
    size_t m_textSize = 0;

    SecureBuffer m_pixelBuffer;

    int m_imageWidth = 0;
    int m_imageHeight = 0;

    QByteArray m_fontData;
    stbtt_fontinfo m_fontInfo;
    bool m_fontLoaded = false;
    float m_fontSize = 14.0f;

    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool m_obfuscated = false; // true = рисовать ••••
};