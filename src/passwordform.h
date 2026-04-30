#pragma once

#include <QWidget>

namespace Ui {
class PasswordForm;
}

class PasswordForm : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordForm(QWidget *parent = nullptr);
    ~PasswordForm();

private slots:
    void on_pushButton_clicked();

private:
    Ui::PasswordForm *ui;
};

