#include "root_view.h"
#include "ui_root_view.h"
#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "storage/database_manager.h"

RootView::RootView(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::RootView)
{
    ui->setupUi(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    SetupView = new class SetupView(stack);
    UnlockView = new class UnlockView(stack);

    connect(SetupView, &SetupView::registerSuccess, this, &RootView::onRegistrationComplete);
    connect(this, &RootView::themeChanged, SetupView, &SetupView::onThemeChanged);

    connect(UnlockView, &UnlockView::loginSuccess, this, &RootView::onLoginSuccess);
    connect(this, &RootView::themeChanged, UnlockView, &UnlockView::onThemeChanged);

    connect(&KeyManager::instance(), &KeyManager::keyCleared, this, &RootView::onKeyCleared);

    stack->addWidget(SetupView);
    stack->addWidget(UnlockView);

    setWidget();
}

RootView::~RootView()
{
    delete ui;
}

void RootView::setWidget()
{
    if (isHashFileExists())
        stack->setCurrentWidget(UnlockView);
    else
        stack->setCurrentWidget(SetupView);
}

void RootView::onRegistrationComplete()
{
    stack->removeWidget(SetupView);
    SetupView->deleteLater();
    SetupView = nullptr;

    stack->setCurrentWidget(UnlockView);
}

void RootView::onLoginSuccess()
{
    QString dbPath = QCoreApplication::applicationDirPath() + "/passDB.sqlite";

    if (!DatabaseManager::instance().openDatabase(dbPath, "TODO"))
    {
        qWarning() << "Failed to open database!";
        return;
    }

    if (!DatabaseManager::instance().createTables())
    {
        qWarning() << "Failed to create tables!";
        return;
    }

    VaultView = new class VaultView(stack);

    connect(VaultView, &VaultView::lockRequested, this, &RootView::onLockRequested);
    connect(this, &RootView::themeChanged, VaultView, &VaultView::onThemeChanged);

    stack->removeWidget(UnlockView);
    UnlockView->deleteLater();
    UnlockView = nullptr;

    stack->addWidget(VaultView);
    stack->setCurrentWidget(VaultView);
}

void RootView::onKeyCleared()
{
    DatabaseManager::instance().closeDatabase();

    stack->removeWidget(VaultView);
    VaultView->deleteLater();
    VaultView = nullptr;

    UnlockView = new class UnlockView(stack);
    connect(UnlockView, &UnlockView::loginSuccess, this, &RootView::onLoginSuccess);
    connect(this, &RootView::themeChanged, UnlockView, &UnlockView::onThemeChanged);

    stack->addWidget(UnlockView);
    stack->setCurrentWidget(UnlockView);
}

void RootView::onLockRequested()
{
    KeyManager::instance().clearKey();
}

bool RootView::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange)
        emit themeChanged();

    return QMainWindow::event(event);
}

