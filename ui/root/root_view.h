#pragma once

#include "setup/setup_view.h"
#include "unlock/unlock_view.h"
#include "vault/vault_view.h"

#include <QMainWindow>
#include <QStackedWidget>

namespace Ui {
class RootView;
}

class RootView : public QMainWindow
{
    Q_OBJECT

public:
    explicit RootView(QWidget *parent = nullptr);
    ~RootView();

private slots:
    void onRegistrationComplete();
    void onLoginSuccess();
    void onKeyCleared();
    void onLockRequested();

private:
    Ui::RootView *ui;

    QStackedWidget *stack;
    SetupView *SetupView;
    UnlockView *UnlockView;
    VaultView *VaultView;

    void setWidget();
};
