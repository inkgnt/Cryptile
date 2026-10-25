#pragma once

#include <QWidget>
#include <QToolButton>


namespace Ui {
    class UnlockView;
}

class UnlockView : public QWidget
{
    Q_OBJECT

public:
    explicit UnlockView(QWidget *parent = nullptr);
    ~UnlockView();

signals:
    void loginSuccess();

private slots:
    void onloginButtonClicked();

private:
    Ui::UnlockView *ui;

    bool isPasswordVisible = false;
    QToolButton *toggleButton = nullptr;
};
