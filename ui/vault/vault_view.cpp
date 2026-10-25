#include "vault_view.h"
#include "ui_vault_view.h"

#include "data_entry_widget.h"
#include "common/data_entry_dialog.h"
#include "common/dialog.h"
#include "utils/theme.h"
#include "storage/database_manager.h"

#include <QLineEdit>


namespace {
    constexpr auto idRole = Qt::UserRole;
    constexpr auto nonSensitiveDataRole = Qt::UserRole + 1;
}


VaultView::VaultView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VaultView)
{
    ui->setupUi(this);

    ui->searchLine->addAction(QIcon(new ThemedIconEngine("search")), QLineEdit::LeadingPosition);
    ui->addEntryButton->setIcon(QIcon(new ThemedIconEngine("add")));
    ui->deleteEntryButton->setIcon(QIcon(new ThemedIconEngine("delete")));
    ui->lockButton->setIcon(QIcon(new ThemedIconEngine("lock")));
    ui->importButton->setIcon(QIcon(new ThemedIconEngine("import")));
    ui->exportButton->setIcon(QIcon(new ThemedIconEngine("export")));
    ui->settingsButton->setIcon(QIcon(new ThemedIconEngine("settings")));

    connect(ui->searchLine, &QLineEdit::textChanged, this, &VaultView::applyFilter);
    connect(ui->addEntryButton, &QPushButton::clicked, this, &VaultView::onAddEntryButtonClicked);
    connect(ui->deleteEntryButton, &QPushButton::clicked, this, &VaultView::onDeleteEntryButtonClicked);
    connect(ui->lockButton, &QPushButton::clicked, this, &VaultView::lockRequested);
    connect(ui->importButton, &QPushButton::clicked, this, &VaultView::onImportButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &VaultView::onExportButtonClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &VaultView::onSettingsButtonClicked);

    loadDataToList();
}

VaultView::~VaultView()
{
    DatabaseManager::instance().closeDatabase();
    delete ui;
}

void VaultView::onAddEntryButtonClicked()
{
    Dialog d (new DataForm, this);
    if (d.exec() == QDialog::Accepted)
        loadDataToList();
}

void VaultView::onDeleteEntryButtonClicked()
{
    QListWidgetItem *selectedItem = ui->listWidget->currentItem();
    if (!selectedItem) return;

    qint64 id = selectedItem->data(Qt::UserRole).toLongLong();

    DatabaseManager::instance().deleteRecord(id);
    loadDataToList();
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

void VaultView::loadDataToList()
{
    ui->listWidget->clear();
    const QList<DataRecord> records = DatabaseManager::instance().getAllRecords();

    for (const DataRecord& record : records)
    {
        auto *widget = new DataEntryWidget(record, this);

        auto *item = new QListWidgetItem(ui->listWidget);
        item->setData(idRole, record.id);
        item->setData(nonSensitiveDataRole, record.url);
        item->setSizeHint(widget->sizeHint());

        ui->listWidget->setItemWidget(item, widget);
    }
}

void VaultView::applyFilter(const QString &filter)
{
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        QListWidgetItem* item = ui->listWidget->item(i);
        bool matches = filter.isEmpty() || item->data(nonSensitiveDataRole).toString().contains(filter, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}
