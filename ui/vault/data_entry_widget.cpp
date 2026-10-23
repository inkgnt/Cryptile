#include "data_entry_widget.h"
#include "ui_data_entry_widget.h"

#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "storage/database_manager.h"

#include <QListWidget>
#include <QMouseEvent>


namespace {
//TODO theme manager class, maybe abstract base class
inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

QListWidget* findlistWidget(QWidget* start)
{
    QWidget *p = start;
    while (p)
    {
        if (auto lw = qobject_cast<QListWidget*>(p))
            return lw;
        p = p->parentWidget();
    }
    return nullptr;
}
}

DataEntryWidget::DataEntryWidget(DataRecord& record, QWidget *parent)
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

    connect(ui->showButton, &QPushButton::clicked, this, &DataEntryWidget::onShowButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DataEntryWidget::onDeleteButtonClicked);

    ui->showButton->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark"));
    ui->deleteButton->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_delete_light" : ":/icons/dark/icon_delete_dark"));
}

DataEntryWidget::~DataEntryWidget()
{
    delete ui;
}

void DataEntryWidget::onShowButtonClicked()
{
    if (isObfuscated)
    {
        std::vector<std::uint8_t> ivLogin; ivLogin.assign(record.login.begin(), record.login.begin() + 16);
        std::vector<std::uint8_t> ivPass; ivPass.assign(record.password.begin(), record.password.begin() + 16);
        std::vector<std::uint8_t> cipherLogin; cipherLogin.assign(record.login.begin() + 16, record.login.end());
        std::vector<std::uint8_t> cipherPass; cipherPass.assign(record.password.begin() + 16, record.password.end());

        auto plainLogin = decryptAES256(cipherLogin, KeyManager::instance().getKey(), ivLogin);
        auto plainPass = decryptAES256(cipherPass, KeyManager::instance().getKey(), ivPass);

        QString loginString = QString::fromUtf8(reinterpret_cast<const char*>(plainLogin.data()), plainLogin.size());
        QString passString = QString::fromUtf8(reinterpret_cast<const char*>(plainPass.data()), plainPass.size());

        ui->loginLabel->setText("********");
        ui->passwordLabel->setText("********");
        isObfuscated = false;

        ui->loginLabel->setText(loginString);
        ui->passwordLabel->setText(passString);

        ui->showButton->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark"));
    }
    else
    {
        ui->loginLabel->setText("********");
        ui->passwordLabel->setText("********");
        isObfuscated = true;

        ui->showButton->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark"));
    }
}

void DataEntryWidget::onDeleteButtonClicked()
{
    QListWidget *listWidget = findlistWidget(this);
    if (!listWidget)
        return;

    QListWidgetItem *foundItem = nullptr;

    for (int i = 0; i < listWidget->count(); ++i)
    {
        QListWidgetItem *item = listWidget->item(i);
        if (listWidget->itemWidget(item) == this)
        {
            foundItem = item;
            break;
        }
    }

    if (foundItem)
    {
        int row = listWidget->row(foundItem);
        listWidget->takeItem(row);
        delete foundItem;

        DatabaseManager::instance().deleteRecord(record.id);
    }

    emit syncRequested();
}

void DataEntryWidget::onThemeChanged()
{
    if(this->palette().color(QPalette::Window).lightness() < 128)
    {
        ui->showButton->setIcon(QIcon(isObfuscated ? ":/icons/light/icon_show_light" : ":/icons/light/icon_hide_light"));

        ui->deleteButton->setIcon(QIcon(":/icons/light/icon_delete_light"));
    }
    else
    {
        ui->showButton->setIcon(QIcon(isObfuscated ? ":/icons/dark/icon_show_dark" : ":/icons/dark/icon_hide_dark"));

        ui->deleteButton->setIcon(QIcon(":/icons/dark/icon_delete_dark"));
    }
}
