#pragma once

#include <QDialog>
#include <QVBoxLayout>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *childWidget, QWidget *parent = nullptr);
    ~Dialog();

private:
    Ui::Dialog *ui;

    QVBoxLayout *layout;
};
