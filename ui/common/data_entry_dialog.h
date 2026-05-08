#pragma once

#include <QWidget>

namespace Ui {
class DataForm;
}

class DataForm : public QWidget
{
    Q_OBJECT

public:
    explicit DataForm(QWidget *parent = nullptr);
    ~DataForm();

private slots:
    void on_pushButton_clicked();

private:
    Ui::DataForm *ui;
};

