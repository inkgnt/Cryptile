#pragma once

#include <QFrame>
#include <QByteArray>
#include "utils/secure_buffer.h"
#include <stb_truetype.h>

class SecureLineEdit : public QFrame
{
    Q_OBJECT

    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool obfuscated READ isObfuscated WRITE setObfuscated)
    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)

public:
    explicit SecureLineEdit(QWidget *parent = nullptr);
    ~SecureLineEdit() = default;

    SecureBuffer getSecureText() const;
    void clear();

    Qt::Alignment alignment() const;
    void setAlignment(Qt::Alignment align);

    bool isObfuscated() const;
    void setObfuscated(bool obfuscate);

    QString placeholderText() const;
    void setPlaceholderText(const QString& placeholder);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void loadFont();
    void renderTextToPixels();
    void clearCanvas();
    QRect textRect() const;

    int totalChars() const;
    int xToCharIndex(int targetX) const;
    size_t charIndexToByteOffset(int targetIdx) const;

    void insertText(const SecureBuffer& utf8);
    bool deleteSelectedText();

    SecureBuffer m_textBuffer;
    size_t m_textCapacity = 0;
    size_t m_textLen = 0;

    int m_cursorCharIdx = 0;
    int m_selectionStartCharIdx = 0;

    SecureBuffer m_pixelBuffer;
    size_t m_pixelBufferCapacity = 0;
    SecureBuffer m_glyphBuffer;

    int m_imageWidth = 0;
    int m_imageHeight = 0;
    int m_logicalWidth = 0;
    int m_logicalHeight = 0;

    int m_logicalCursorX = 0;
    int m_logicalCursorY = 0;
    int m_logicalSelStartX = 0;
    int m_logicalSelEndX = 0;

    QRect m_logicalTextRect;

    int m_cursorTimerId = 0;
    bool m_cursorVisible = false;
    bool m_needsRender = true;

    QByteArray m_fontData;
    stbtt_fontinfo m_fontInfo;
    bool m_fontLoaded = false;
    float m_fontSize = 14.0f;

    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool m_obfuscated = false;
    QString m_placeholderText;
};