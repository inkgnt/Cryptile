#include "vault_view.h"
#include "ui_vault_view.h"

#include "data_entry_widget.h"
#include "common/data_entry_dialog.h"
#include "common/dialog.h"
#include "utils/theme.h"
#include "storage/database_manager.h"

#include <QLineEdit>

VaultView::VaultView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VaultView)
{
    ui->setupUi(this);

    ui->searchLine->addAction(QIcon(new ThemedIconEngine("search")), QLineEdit::LeadingPosition);
    ui->addPasswordButton->setIcon(QIcon(new ThemedIconEngine("add")));
    ui->deletePasswordButton->setIcon(QIcon(new ThemedIconEngine("delete")));
    ui->lockButton->setIcon(QIcon(new ThemedIconEngine("lock")));
    ui->importButton->setIcon(QIcon(new ThemedIconEngine("import")));
    ui->exportButton->setIcon(QIcon(new ThemedIconEngine("export")));
    ui->settingsButton->setIcon(QIcon(new ThemedIconEngine("settings")));

    loadDataToList(QString());

    connect(ui->searchLine, &QLineEdit::textChanged, this, &VaultView::loadDataToList);
    connect(ui->addPasswordButton, &QPushButton::clicked, this, &VaultView::onAddPasswordButtonClicked);
    connect(ui->deletePasswordButton, &QPushButton::clicked, this, &VaultView::onDeletePasswordButtonClicked);
    connect(ui->lockButton, &QPushButton::clicked, this, &VaultView::onLockButtonClicked); //TODO lambda
    connect(ui->importButton, &QPushButton::clicked, this, &VaultView::onImportButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &VaultView::onExportButtonClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &VaultView::onSettingsButtonClicked);

    ui->searchLine->setPlaceholderText("Search:");
}

VaultView::~VaultView()
{
    DatabaseManager::instance().closeDatabase();
    delete ui;
}

void VaultView::onAddPasswordButtonClicked()
{
    auto pw = new DataForm;
    auto d = new Dialog(pw, this);
    d->exec();

    loadDataToList(QString());
}

void VaultView::onDeletePasswordButtonClicked()
{
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) return;

    int id = selectedItem->data(Qt::UserRole).toInt();

    if (DatabaseManager::instance().deleteRecord(id)) {
        int row = ui->listWidget->row(selectedItem);
        QListWidgetItem *item = ui->listWidget->takeItem(row);
        delete item;
    }

    loadDataToList(QString());
}

void VaultView::onLockButtonClicked()
{
    emit lockRequested();
}

void VaultView::onImportButtonClicked()
{
    //
}

void VaultView::onExportButtonClicked()
{
    //
}

void VaultView::onSettingsButtonClicked()
{
    //
}

void VaultView::loadDataToList(const QString& filter)
{
    ui->listWidget->clear();
    QList<DataRecord> records = DatabaseManager::instance().getAllRecords();

    for (DataRecord& record : records)
    {
        if (!(filter.isEmpty()) )
            continue;

        auto *widget = new DataEntryWidget(record, this);

        //TODO
        connect(widget, &DataEntryWidget::syncRequested, this, &VaultView::onSyncRequested);

        auto *item = new QListWidgetItem(ui->listWidget);
        item->setData(Qt::UserRole, record.id);
        item->setSizeHint(widget->sizeHint());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);
    }
}

void VaultView::onSyncRequested()
{
    loadDataToList(QString());
}
