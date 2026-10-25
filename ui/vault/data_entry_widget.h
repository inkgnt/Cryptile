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
    explicit DataEntryWidget(const DataRecord& record, QWidget *parent = nullptr);

    ~DataEntryWidget();

private slots:
    void onShowButtonToggled(bool visible);

private:
    Ui::DataEntryWidget *ui;

    DataRecord record;

    bool obfuscated = true;
};
