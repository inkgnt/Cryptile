#include "unlock_view.h"
#include "ui_unlock_view.h"

#include "crypto/crypto.h"
#include "keymanager/keymanager.h"
#include "utils/theme.h"

#include <QFile>
#include <QMessageBox>
#include <QToolButton>
#include <QHBoxLayout>


UnlockView::UnlockView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UnlockView)
{
    ui->setupUi(this);

    ui->passwordLineEdit->setEchoMode(QLineEdit::Password);

    toggleButton = new QToolButton(ui->passwordLineEdit);
    toggleButton->setCursor(Qt::ArrowCursor);
    toggleButton->setCheckable(true);
    toggleButton->setIcon(QIcon(new ThemedIconEngine("show", "hide")));
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

    connect(toggleButton, &QToolButton::toggled, this, [this](bool visible) {
        ui->passwordLineEdit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
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
