#include "registerwidget.h"
#include "ui_registerwidget.h"

#include <QMessageBox>
#include "crypto.h"
#include <QToolButton>

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

RegisterWidget::RegisterWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterWidget)
{
    ui->setupUi(this);
    ui->PSWDlineEdit->setEchoMode(QLineEdit::Password);
    ui->PSWDREPEATlineEdit->setEchoMode(QLineEdit::Password);

    QString showIcon = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";

    toggleButton1 = new QToolButton(ui->PSWDlineEdit);
    toggleButton2 = new QToolButton(ui->PSWDREPEATlineEdit);

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

    QHBoxLayout *layout = new QHBoxLayout(ui->PSWDlineEdit);
    QHBoxLayout *layoutRepeat = new QHBoxLayout(ui->PSWDREPEATlineEdit);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(toggleButton1);

    layoutRepeat->setContentsMargins(0, 0, 0, 0);
    layoutRepeat->addStretch();
    layoutRepeat->addWidget(toggleButton2);

    ui->PSWDlineEdit->setLayout(layout);
    ui->PSWDREPEATlineEdit->setLayout(layoutRepeat);

    connect(toggleButton1, &QToolButton::clicked, this, [this]() {
        isFirstPasswordVisible = !isFirstPasswordVisible;

        ui->PSWDlineEdit->setEchoMode(isFirstPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isFirstPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton1->setIcon(QIcon(iconPath));
    });

    connect(toggleButton2, &QToolButton::clicked, this, [this]() {
        isSecondPasswordVisible = !isSecondPasswordVisible;

        ui->PSWDREPEATlineEdit->setEchoMode(isSecondPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isSecondPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton2->setIcon(QIcon(iconPath));
    });

    connect(ui->SUBMITbtn, &QPushButton::clicked, this, &RegisterWidget::onSUBMITbtnClicked);
}

RegisterWidget::~RegisterWidget()
{
    delete ui;
}

void RegisterWidget::onSUBMITbtnClicked()
{
    QString password = ui->PSWDlineEdit->text();
    QString confirmPassword = ui->PSWDREPEATlineEdit->text();

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

    ui->PSWDlineEdit->clear();
    ui->PSWDREPEATlineEdit->clear();

    emit registerSuccess();
}

void RegisterWidget::onThemeChanged() {
    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";
    QString hide = isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark";

    toggleButton1->setIcon(QIcon(isFirstPasswordVisible ? hide : show));
    toggleButton2->setIcon(QIcon(isSecondPasswordVisible ? hide : show));
}
