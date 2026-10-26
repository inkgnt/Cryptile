#include "unlock_view.h"
#include "ui_unlock_view.h"

#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "common/widget_utils.h"

#include <QMessageBox>


UnlockView::UnlockView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UnlockView)
{
    ui->setupUi(this);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    addVisibilityToggle(ui->passwordLineEdit);

    connect(ui->loginButton, &QPushButton::clicked, this, &UnlockView::onloginButtonClicked);
}

UnlockView::~UnlockView()
{
    delete ui;
}

void UnlockView::onloginButtonClicked()
{
    QString password = ui->passwordLineEdit->text();
    if (password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Password cannot be empty!");
        return;
    }

    std::vector<std::uint8_t> storedSalt;
    std::vector<std::uint8_t> storedHash;

    loadHashAndSaltFromFile(storedSalt, storedHash);

    if (memcmp(generatePBKDF2Hash(password, storedSalt).data(), storedHash.data(), storedHash.size()) == 0)
    {
        KeyManager::instance().setKey(generateScryptKey(password, storedSalt));

        ui->passwordLineEdit->clear();
        emit loginSuccess();
    }
    else
    {
        QMessageBox::warning(this, "Error", "Password is wrong!");
        ui->passwordLineEdit->clear();
    }
}
