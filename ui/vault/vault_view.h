#pragma once

#include <QWidget>


namespace Ui {
    class VaultView;
}

class VaultView : public QWidget
{
    Q_OBJECT

public:
    explicit VaultView(QWidget *parent = nullptr);
    ~VaultView();

signals:
    void lockRequested();

private slots:
    void onAddEntryButtonClicked();
    void onDeleteEntryButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
    void onSettingsButtonClicked();
    void applyFilter(const QString &filter);

private:
    Ui::VaultView *ui;

    QString m_filter;

    void loadDataToList();
};
