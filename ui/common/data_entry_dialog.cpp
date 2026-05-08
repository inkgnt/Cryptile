#include "data_entry_dialog.h"
#include "ui_data_entry_dialog.h"

#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "storage/database_manager.h"
#include <QMessageBox>

DataForm::DataForm(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataForm)
{
    ui->setupUi(this);
}

DataForm::~DataForm()
{
    delete ui;
}

void DataForm::on_pushButton_clicked()
{
    auto url = ui->lineEdit->text();
    QByteArray loginQA = ui->usernameLineEdit->text().toUtf8();
    QByteArray passwordQA = ui->lineEdit_3->text().toUtf8();

    std::vector<uint8_t> login(loginQA.begin(), loginQA.end());
    std::vector<uint8_t> password(passwordQA.begin(), passwordQA.end());

    auto ivLogin = generateIV();
    auto ivPassword = generateIV();

    auto cipherLog = encryptAES256(login, KeyManager::instance().getKey(), ivLogin);
    auto cipherPass = encryptAES256(password, KeyManager::instance().getKey(), ivPassword);

    QByteArray logDb, passDb;
    logDb.append(ivLogin).append(cipherLog);
    passDb.append(ivPassword).append(cipherPass);

    if (DatabaseManager::instance().addRecord(url, logDb, passDb))
    {
        QMessageBox::information(this, "Ok", "Record added successfully.");
        if (auto dlg = qobject_cast<QDialog*>(this->parentWidget()))
        {
            dlg->accept();
        }
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to add record to database.");
        if (auto dlg = qobject_cast<QDialog*>(this->parentWidget()))
        {
            dlg->reject();
        }
    }
}

