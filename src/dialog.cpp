#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(QWidget *childWidget, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
    layout = new QVBoxLayout(this);

    layout->addWidget(childWidget);
    //childWidget->setParent(this);
    setLayout(layout);

    //connect(childWidget, &QWidget::destroyed, this, &QDialog::accept);
    //connect(childWidget, &QWidget::destroyed, this, &QDialog::reject);
}

Dialog::~Dialog()
{
    delete layout;
    layout = nullptr;
    delete ui;
}
