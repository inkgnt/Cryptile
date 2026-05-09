#pragma once

#include "storage/database_manager.h"

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

public slots:
    void onThemeChanged();

private slots:
    void onShowButtonClicked();

    void onDeleteButtonClicked();

private:
    Ui::DataEntryWidget *ui;

    DataRecord record;
    //bool eventFilter(QObject *obj, QEvent *event);

    bool flag = true;
};
