#include "setup_view.h"
#include "ui_setup_view.h"

#include "crypto/crypto.h"
#include "common/widget_utils.h"

#include <QMessageBox>


SetupView::SetupView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SetupView)
{
    ui->setupUi(this);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

    addVisibilityToggle(ui->passwordLineEdit);
    addVisibilityToggle(ui->confirmPasswordLineEdit);

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
