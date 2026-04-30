#include "loginwidget.h"
#include "ui_loginwidget.h"
#include <QFile>

#include <QMessageBox>
#include "crypto.h"
#include "keymanager.h"
#include <QToolButton>
#include <QToolButton>
#include <QHBoxLayout>
inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginWidget)
{
    ui->setupUi(this);

    QString showIcon = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";

    ui->PSWDlineEdit->setEchoMode(QLineEdit::Password);

    toggleButton = new QToolButton(ui->PSWDlineEdit);
    toggleButton->setCursor(Qt::ArrowCursor);
    toggleButton->setIcon(QIcon(showIcon));
    toggleButton->setFixedSize(25, 25);
    toggleButton->setStyleSheet(R"(
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
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(toggleButton);
    ui->PSWDlineEdit->setLayout(layout);

    connect(toggleButton, &QToolButton::clicked, this, [this]() {
        isPasswordVisible = !isPasswordVisible;

        ui->PSWDlineEdit->setEchoMode(isPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton->setIcon(QIcon(iconPath));
    });

    connect(ui->LOGINbtn, &QPushButton::clicked, this, &LoginWidget::onLOGINbtnClicked);
}

LoginWidget::~LoginWidget()
{
    delete ui;
}

void LoginWidget::onLOGINbtnClicked()
{
    QString password = ui->PSWDlineEdit->text();
    if (password.isEmpty())
    {
        QMessageBox::warning(this, "Error", "Password cannot be empty!");
        return;
    }

    std::vector<uint8_t> storedSalt;
    std::vector<uint8_t> storedHash;

    loadHashAndSaltFromFile(storedSalt, storedHash);

    if (memcmp(generatePBKDF2Hash(password, storedSalt).data(), storedHash.data(), storedHash.size()) == 0)
    {
        KeyManager::instance().setKey(generateScryptKey(password, storedSalt));

        ui->PSWDlineEdit->clear();
        emit loginSuccess();
    }
    else
    {
        QMessageBox::warning(this, "Error", "Password is wrong!");
        ui->PSWDlineEdit->clear();
    }
}

void LoginWidget::onThemeChanged() {
    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";
    QString hide = isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark";

    toggleButton->setIcon(QIcon(isPasswordVisible ? hide : show));
}
