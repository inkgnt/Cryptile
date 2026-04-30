#pragma once

#include <QMainWindow>
#include <QStackedWidget>

#include "registerwidget.h"
#include "loginwidget.h"
#include "mainwindowwidget.h"

namespace Ui {
class MainApp;
}

class MainApp : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainApp(QWidget *parent = nullptr);
    ~MainApp();

signals:
    void themeChanged();

private slots:
    void onRegistrationComplete();
    void onLoginSuccess();
    void onKeyCleared();
    void onLockRequested();

private:
    Ui::MainApp *ui;

    QStackedWidget *stack;
    RegisterWidget *registerWidget;
    LoginWidget *loginWidget;
    MainWindowWidget *mainWindowWidget;

    void setWidget();
protected:
    bool event(QEvent *event) override;
};
