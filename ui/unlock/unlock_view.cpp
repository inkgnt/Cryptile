#include "unlock_view.h"
#include "ui_unlock_view.h"


#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include <QFile>

#include <QMessageBox>
#include <QToolButton>
#include <QToolButton>
#include <QHBoxLayout>

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

UnlockView::UnlockView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UnlockView)
{
    ui->setupUi(this);

    QString showIcon = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    toggleButton = new QToolButton(ui->passwordLineEdit);
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

    QHBoxLayout *layout = new QHBoxLayout(ui->passwordLineEdit);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(toggleButton);
    ui->passwordLineEdit->setLayout(layout);

    connect(toggleButton, &QToolButton::clicked, this, [this]() {
        isPasswordVisible = !isPasswordVisible;

        ui->passwordLineEdit->setEchoMode(isPasswordVisible ? QLineEdit::Normal : QLineEdit::Password);

        QString iconPath = isPasswordVisible
                               ? (isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark")
                               : (isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark");

        toggleButton->setIcon(QIcon(iconPath));
    });

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

    std::vector<uint8_t> storedSalt;
    std::vector<uint8_t> storedHash;

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

void UnlockView::onThemeChanged() {
    QString show = isDarkTheme(this) ? ":/icons/light/icon_show_light" : ":/icons/dark/icon_show_dark";
    QString hide = isDarkTheme(this) ? ":/icons/light/icon_hide_light" : ":/icons/dark/icon_hide_dark";

    toggleButton->setIcon(QIcon(isPasswordVisible ? hide : show));
}
