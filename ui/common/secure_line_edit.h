#pragma once
#include "secure_text_widget.h"

#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QTimerEvent>

class SecureLineEdit : public SecureTextWidget {
    Q_OBJECT
public:
    explicit SecureLineEdit(QWidget *parent = nullptr);
    ~SecureLineEdit() override = default;

    void insertText(const SecureBuffer& utf8);
    bool deleteSelectedText();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void ensureCursorVisible();
    void resizeEvent(QResizeEvent *event) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void timerEvent(QTimerEvent *event) override;

private:
    int m_cursorTimerId = 0;

    int m_autoScrollTimerId = 0;
    int m_autoScrollDirection = 0;
};