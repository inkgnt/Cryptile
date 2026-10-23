#pragma once

#include "storage/data_record.h"
#include <QWidget>

namespace Ui {
class DataEntryWidget;
}

class DataEntryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DataEntryWidget(DataRecord& record, QWidget *parent = nullptr);

    ~DataEntryWidget();

signals:
    void syncRequested();

private slots:
    void onShowButtonClicked();

    void onDeleteButtonClicked();

private:
    Ui::DataEntryWidget *ui;

    DataRecord record;

    bool obfuscated = true;
};
