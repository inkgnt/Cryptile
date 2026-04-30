#pragma once

#include <QWidget>

namespace Ui {
class LoginWidget;
}

class LoginWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWidget(QWidget *parent = nullptr);
    ~LoginWidget();

signals:
    void loginSuccess();

public slots:
    void onThemeChanged();

private slots:
    void onLOGINbtnClicked();

private:
    Ui::LoginWidget *ui;

    bool isPasswordVisible = false;
    QToolButton *toggleButton = nullptr;
};
