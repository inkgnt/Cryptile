#include "setup_view.h"
#include "ui_setup_view.h"

#include "crypto/crypto.h"

#include <QMessageBox>
#include <QToolButton>

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

SetupView::SetupView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetupView)
{
    ui->setupUi(this);
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    QString showIcon = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";

    toggleButton1 = new QToolButton(ui->passwordLineEdit);
    toggleButton2 = new QToolButton(ui->confirmPasswordLineEdit);

    toggleButton1->setCursor(Qt::ArrowCursor);
    toggleButton1->setIcon(QIcon(showIcon));
    toggleButton1->setFixedSize(25, 25);
    toggleButton1->setStyleSheet(R"(
        QToolButton {
            border: none;
            background-color: transparent;
        }
        QToolButton:hover {
            background-color: rgba(150, 150, 150, 50);
            border-radius: 4px;
        }
    )");

    toggleButton2->setCursor(Qt::ArrowCursor);
    toggleButton2->setIcon(QIcon(showIcon));
    toggleButton2->setFixedSize(25, 25);
    toggleButton2->setStyleSheet(R"(
        QToolButton {
            border: none;
            background-color: transparent;
        }
        QToolButton:hover {
            background-color: rgba(150, 150, 150, 50);
            border-radius: 4px;
        }
    )");

    QHBoxLayout *layout = new QHBoxLayout(ui->passwordLineEdit);
    QHBoxLayout *layoutRepeat = new QHBoxLayout(ui->confirmPasswordLineEdit);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(toggleButton1);

    layoutRepeat->setContentsMargins(0, 0, 0, 0);
    layoutRepeat->addStretch();
    layoutRepeat->addWidget(toggleButton2);

    ui->passwordLineEdit->setLayout(layout);
    ui->confirmPasswordLineEdit->setLayout(layoutRepeat);

    connect(toggleButton1, &QToolButton::clicked, this, [this]() {
        isFirstPasswordVisible = !isFirstPasswordVisible;

        ui->passwordLineEdit->setEchoMode(isFirstPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isFirstPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton1->setIcon(QIcon(iconPath));
    });

    connect(toggleButton2, &QToolButton::clicked, this, [this]() {
        isSecondPasswordVisible = !isSecondPasswordVisible;

        ui->confirmPasswordLineEdit->setEchoMode(isSecondPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isSecondPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton2->setIcon(QIcon(iconPath));
    });

    connect(ui->submitButton, &QPushButton::clicked, this, &SetupView::onsubmitButtonClicked);
}

SetupView::~SetupView()
{
    delete ui;
}

void SetupView::onsubmitButtonClicked()
{
    QString password = ui->passwordLineEdit->text();
    QString confirmPassword = ui->confirmPasswordLineEdit->text();

    if (password.isEmpty() || confirmPassword.isEmpty())
    {
        QMessageBox::warning(this, "Error", "All fields must be filled in!");
        return;
    }

    if (password != confirmPassword)
    {
        QMessageBox::warning(this, "Error", "The passwords do not match!");
        return;
    }

    auto salt = generateSalt();
    auto hash = generatePBKDF2Hash(password, salt);

    saveHashAndSaltToFile(salt, hash);

    ui->passwordLineEdit->clear();
    ui->confirmPasswordLineEdit->clear();

    emit registerSuccess();
}

void SetupView::onThemeChanged() {
    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";
    QString hide = isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark";

    toggleButton1->setIcon(QIcon(isFirstPasswordVisible ? hide : show));
    toggleButton2->setIcon(QIcon(isSecondPasswordVisible ? hide : show));
}
