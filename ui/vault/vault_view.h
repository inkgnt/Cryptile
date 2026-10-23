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

    void onAddPasswordButtonClicked();
    void onDeletePasswordButtonClicked();
    void onLockButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
    void onSettingsButtonClicked();

    void onSyncRequested();
    void loadDataToList(const QString &text);

private:
    Ui::VaultView *ui;
};
