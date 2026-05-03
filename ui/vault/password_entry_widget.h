#pragma once

#include "storage/database_manager.h"

#include <QWidget>

namespace Ui {
class PasswordEntryWidget;
}

class PasswordEntryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PasswordEntryWidget(DataRecord& record, QWidget *parent = nullptr);

    ~PasswordEntryWidget();

signals:
    void syncRequested();

public slots:
    void onThemeChanged();

private slots:
    void onShowButtonClicked();

    void onDeleteButtonClicked();

private:
    Ui::PasswordEntryWidget *ui;

    DataRecord record;
    bool eventFilter(QObject *obj, QEvent *event);

    bool flag = true;
};
