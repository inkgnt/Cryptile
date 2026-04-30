#include "mainapp.h"
#include "ui_mainapp.h"
#include "crypto.h"
#include "keymanager.h"
#include "databasemanager.h"

MainApp::MainApp(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainApp)
{
    ui->setupUi(this);

    stack = new QStackedWidget(this);
    setCentralWidget(stack);

    registerWidget = new RegisterWidget(stack);
    loginWidget = new LoginWidget(stack);

    connect(registerWidget, &RegisterWidget::registerSuccess, this, &MainApp::onRegistrationComplete);
    connect(this, &MainApp::themeChanged, registerWidget, &RegisterWidget::onThemeChanged);

    connect(loginWidget, &LoginWidget::loginSuccess, this, &MainApp::onLoginSuccess);
    connect(this, &MainApp::themeChanged, loginWidget, &LoginWidget::onThemeChanged);

    connect(&KeyManager::instance(), &KeyManager::keyCleared, this, &MainApp::onKeyCleared);

    stack->addWidget(registerWidget);
    stack->addWidget(loginWidget);

    setWidget();
}

MainApp::~MainApp()
{
    delete ui;
}

void MainApp::setWidget()
{
    if (isHashFileExists())
        stack->setCurrentWidget(loginWidget);
    else
        stack->setCurrentWidget(registerWidget);
}

void MainApp::onRegistrationComplete()
{
    stack->removeWidget(registerWidget);
    registerWidget->deleteLater();
    registerWidget = nullptr;

    stack->setCurrentWidget(loginWidget);
}

void MainApp::onLoginSuccess()
{
    QString dbPath = QCoreApplication::applicationDirPath() + "/passDB.sqlite";

    if (!DatabaseManager::instance().openDatabase(dbPath))
    {
        qWarning() << "Failed to open database!";
        return;
    }

    if (!DatabaseManager::instance().createTables())
    {
        qWarning() << "Failed to create tables!";
        return;
    }

    mainWindowWidget = new MainWindowWidget(stack);

    connect(mainWindowWidget, &MainWindowWidget::lockRequested, this, &MainApp::onLockRequested);
    connect(this, &MainApp::themeChanged, mainWindowWidget, &MainWindowWidget::onThemeChanged);

    stack->removeWidget(loginWidget);
    loginWidget->deleteLater();
    loginWidget = nullptr;

    stack->addWidget(mainWindowWidget);
    stack->setCurrentWidget(mainWindowWidget);
}

void MainApp::onKeyCleared()
{
    DatabaseManager::instance().closeDatabase();

    stack->removeWidget(mainWindowWidget);
    mainWindowWidget->deleteLater();
    mainWindowWidget = nullptr;

    loginWidget = new LoginWidget(stack);
    connect(loginWidget, &LoginWidget::loginSuccess, this, &MainApp::onLoginSuccess);
    connect(this, &MainApp::themeChanged, loginWidget, &LoginWidget::onThemeChanged);

    stack->addWidget(loginWidget);
    stack->setCurrentWidget(loginWidget);
}

void MainApp::onLockRequested()
{
    KeyManager::instance().clearKey();
}

bool MainApp::event(QEvent *event)
{
    if (event->type() == QEvent::ApplicationPaletteChange)
        emit themeChanged();

    return QMainWindow::event(event);
}

