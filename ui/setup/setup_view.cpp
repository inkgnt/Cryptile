#include "setup_view.h"
#include "ui_setup_view.h"

#include "crypto/crypto.h"
#include "utils/theme.h"

#include <QMessageBox>
#include <QToolButton>


SetupView::SetupView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetupView)
{
    ui->setupUi(this);
    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    toggleButton1 = new QToolButton(ui->passwordLineEdit);
    toggleButton2 = new QToolButton(ui->confirmPasswordLineEdit);

    toggleButton1->setCursor(Qt::ArrowCursor);
    toggleButton1->setCheckable(true);
    toggleButton1->setIcon(QIcon(new ThemedIconEngine("show", "hide")));
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
    toggleButton2->setCheckable(true);
    toggleButton2->setIcon(QIcon(new ThemedIconEngine("show", "hide")));
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

    connect(toggleButton1, &QToolButton::toggled, this, [this](bool visible) {
        ui->passwordLineEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });

    connect(toggleButton2, &QToolButton::toggled, this, [this](bool visible) {
        ui->confirmPasswordLineEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
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
