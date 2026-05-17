#pragma once

#include <QFrame>
#include "utils/secure_buffer.h"
#include "secure_font_renderer.h"

class SecureTextWidget : public QFrame {
    Q_OBJECT

    Q_PROPERTY(QString placeholderText READ placeholderText WRITE setPlaceholderText)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(bool obfuscated READ isObfuscated WRITE setObfuscated)

public:
    explicit SecureTextWidget(QWidget *parent = nullptr);
    virtual ~SecureTextWidget() = default;

    void clear();
    SecureBuffer getSecureText() const;

    QString placeholderText() const { return m_placeholderText; }
    void setPlaceholderText(const QString& placeholder);

    QString fontPath() const { return m_fontPath; }
    void setFontPath(const QString& m_fontPath);

    Qt::Alignment alignment() const { return m_alignment; }
    void setAlignment(Qt::Alignment align);

    bool isObfuscated() const { return m_obfuscated; }
    void setObfuscated(bool obfuscate);

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void initStyleOptionForText(QStyleOptionFrame *opt) const;

    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

    QRect textRect() const;
    int totalChars() const;
    size_t charIndexToByteOffset(int targetIdx) const;


    //replace SecureBuffer with ptr
    SecureBuffer getRenderText(size_t& outLen) const;

    SecureBuffer m_textBuffer;
    size_t m_textLen = 0;
    size_t m_textCapacity = 0;

    QString m_placeholderText;

    float m_fontSize = 12.0f;
    QString m_fontPath = ":/fonts/RobotoMono-Regular";

    float m_scrollOffset = 0.0f;

    Qt::Alignment m_alignment = Qt::AlignLeft | Qt::AlignVCenter;
    bool m_obfuscated = false;

    float m_textStartX = 0;
    float m_textStartY = 0;

    int m_cursorCharIdx = 0;
    int m_selectionStartCharIdx = 0;
    bool m_needsRender = true;
    bool m_cursorVisible = false;

    SecureFontRenderer m_renderer;
};