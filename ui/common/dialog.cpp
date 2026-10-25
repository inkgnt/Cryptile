#include "dialog.h"
#include "ui_dialog.h"


Dialog::Dialog(QWidget *childWidget, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    layout = new QVBoxLayout(this);

    layout->addWidget(childWidget);

    setLayout(layout);
}

Dialog::~Dialog()
{
    delete layout;
    layout = nullptr;
    delete ui;
}
