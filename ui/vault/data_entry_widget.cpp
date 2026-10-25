#include "data_entry_widget.h"
#include "ui_data_entry_widget.h"

#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "utils/theme.h"

#include <QListWidget>
#include <QMouseEvent>


DataEntryWidget::DataEntryWidget(const DataRecord& record, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DataEntryWidget)
    , record(record)
{
    ui->setupUi(this);

    ui->urlLabel->setText(record.url);
    ui->loginLabel->setText("********");
    ui->passwordLabel->setText("********");

    ui->urlLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->loginLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->passwordLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui->showButton->setCheckable(true);
    ui->showButton->setIcon(QIcon(new ThemedIconEngine("show", "hide")));

    connect(ui->showButton, &QPushButton::toggled, this, &DataEntryWidget::onShowButtonToggled);
}

DataEntryWidget::~DataEntryWidget()
{
    delete ui;
}

void DataEntryWidget::onShowButtonToggled(bool visible)
{
    if (visible)
    {
        std::vector<std::uint8_t> ivLogin; ivLogin.assign(record.login.begin(), record.login.begin() + 16);
        std::vector<std::uint8_t> ivPass; ivPass.assign(record.password.begin(), record.password.begin() + 16);
        std::vector<std::uint8_t> cipherLogin; cipherLogin.assign(record.login.begin() + 16, record.login.end());
        std::vector<std::uint8_t> cipherPass; cipherPass.assign(record.password.begin() + 16, record.password.end());

        auto plainLogin = decryptAES256(cipherLogin, KeyManager::instance().getKey(), ivLogin);
        auto plainPass = decryptAES256(cipherPass, KeyManager::instance().getKey(), ivPass);

        QString loginString = QString::fromUtf8(reinterpret_cast<const char*>(plainLogin.data()), plainLogin.size());
        QString passString = QString::fromUtf8(reinterpret_cast<const char*>(plainPass.data()), plainPass.size());

        ui->loginLabel->setText(loginString);
        ui->passwordLabel->setText(passString);
    }
    else
    {
        ui->loginLabel->setText("********");
        ui->passwordLabel->setText("********");
    }
}
