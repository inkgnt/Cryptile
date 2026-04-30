#include "passwordcardwidget.h"
#include "ui_passwordcardwidget.h"

#include "crypto.h"
#include "keymanager.h"

#include <QListWidget>
#include <QMouseEvent>

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

PasswordCardWidget::PasswordCardWidget(PasswordRecord& record, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PasswordCardWidget)
    , record(record)
{
    ui->setupUi(this);

    ui->urlLabel->setText("🌐: " + record.url);
    ui->loginLabel->setText("👤: " + QString(8, QChar(0x25CF)));
    ui->passwordLabel->setText("🔒: " + QString(8, QChar(0x25CF)));

    ui->urlLabel->installEventFilter(this);
    ui->loginLabel->installEventFilter(this);
    ui->passwordLabel->installEventFilter(this);

    connect(ui->SHOWbtn, &QPushButton::clicked, this, &PasswordCardWidget::onSHOWbtnClicked);
    connect(ui->DELbtn, &QPushButton::clicked, this, &PasswordCardWidget::onDELbtnClicked);


    ui->SHOWbtn->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark"));
    ui->DELbtn->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_delete_light" : ":/icons/dark/icon_delete_dark"));
}

PasswordCardWidget::~PasswordCardWidget()
{
    delete ui;
}

bool PasswordCardWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        if (obj == ui->urlLabel || obj == ui->loginLabel || obj == ui->passwordLabel)
        {
            QWidget::mousePressEvent(static_cast<QMouseEvent *>(event));
            return false;
        }
    }

    return QWidget::eventFilter(obj, event);
}

QListWidget* findListWidget(QWidget* start)
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

void PasswordCardWidget::onSHOWbtnClicked()
{
    if (flag)
    {
        std::vector<uint8_t> ivLogin; ivLogin.assign(record.login.begin(), record.login.begin() + 16);
        std::vector<uint8_t> ivPass; ivPass.assign(record.password.begin(), record.password.begin() + 16);
        std::vector<uint8_t> cipherLogin; cipherLogin.assign(record.login.begin() + 16, record.login.end());
        std::vector<uint8_t> cipherPass; cipherPass.assign(record.password.begin() + 16, record.password.end());

        auto plainLogin = decryptAES256(cipherLogin, KeyManager::instance().getKey(), ivLogin);
        auto plainPass = decryptAES256(cipherPass, KeyManager::instance().getKey(), ivPass);

        QString loginString = QString::fromUtf8(reinterpret_cast<const char*>(plainLogin.data()), plainLogin.size());
        QString passString = QString::fromUtf8(reinterpret_cast<const char*>(plainPass.data()), plainPass.size());

        ui->loginLabel->setText("👤: " + loginString);
        ui->passwordLabel->setText("🔒: " + passString);
        flag = false;

        ui->SHOWbtn->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark"));
    }
    else
    {
        ui->loginLabel->setText("👤: " + QString(8, QChar(0x25CF)));
        ui->passwordLabel->setText("🔒: " + QString(8, QChar(0x25CF)));
        flag = true;

        ui->SHOWbtn->setIcon(QIcon(isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark"));
    }
}

void PasswordCardWidget::onDELbtnClicked()
{
    QListWidget *listWidget = findListWidget(this);
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

void PasswordCardWidget::onThemeChanged()
{
    if(this->palette().color(QPalette::Window).lightness() < 128)
    {
        ui->SHOWbtn->setIcon(QIcon(flag ? ":/icons/light/icon_show_light" : ":/icons/light/icon_hide_light"));

        ui->DELbtn->setIcon(QIcon(":/icons/light/icon_delete_light"));
    }
    else
    {
        ui->SHOWbtn->setIcon(QIcon(flag ? ":/icons/dark/icon_show_dark" : ":/icons/dark/icon_hide_dark"));

        ui->DELbtn->setIcon(QIcon(":/icons/dark/icon_delete_dark"));
    }
}
