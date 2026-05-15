#include "vault_view.h"
#include "ui_vault_view.h"

#include "data_entry_widget.h"
#include "common/data_entry_dialog.h"
#include "common/dialog.h"

#include <QLineEdit>

#include "storage/database_manager.h"

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

VaultView::VaultView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VaultView)
{
    ui->setupUi(this);

    changeIcons();

    loadDataToList("");

    connect(ui->searchLine, &QLineEdit::textChanged, this, &VaultView::onSearchLineTextChanged);
    connect(ui->addPasswordButton, &QPushButton::clicked, this, &VaultView::onAddPasswordButtonClicked);
    connect(ui->deletePasswordButton, &QPushButton::clicked, this, &VaultView::onDeletePasswordButtonClicked);
    connect(ui->lockButton, &QPushButton::clicked, this, &VaultView::onLockButtonClicked);
    connect(ui->importButton, &QPushButton::clicked, this, &VaultView::onImportButtonClicked);
    connect(ui->exportButton, &QPushButton::clicked, this, &VaultView::onExportButtonClicked);
    connect(ui->settingsButton, &QPushButton::clicked, this, &VaultView::onSettingsButtonClicked);

    ui->frame->setPlaceholderText("placeholderText");
}

VaultView::~VaultView()
{
    DatabaseManager::instance().closeDatabase();
    delete ui;
}

void VaultView::onSearchLineTextChanged(const QString &filter)
{
    loadDataToList(filter);
}

void VaultView::onAddPasswordButtonClicked()
{
    auto pw = new DataForm;
    auto d = new Dialog(pw, this);
    d->exec();

    loadDataToList("");
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

    loadDataToList("");
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

void VaultView::loadDataToList(const QString &filter)
{
    ui->listWidget->clear();
    QList<DataRecord> records = DatabaseManager::instance().getAllRecords();

    for (DataRecord &record : records)
    {
        if (!(filter.isEmpty() || record.url.contains(filter, Qt::CaseInsensitive)))
            continue;

        auto *widget = new DataEntryWidget(record, this);

        connect(widget, &DataEntryWidget::syncRequested, this, &VaultView::onSyncRequested);
        connect(this, &VaultView::themeChanged, widget, &DataEntryWidget::onThemeChanged);

        auto *item = new QListWidgetItem(ui->listWidget);
        item->setData(Qt::UserRole, record.id);
        item->setSizeHint(widget->sizeHint());

        ui->listWidget->addItem(item);
        ui->listWidget->setItemWidget(item, widget);
    }
}

void VaultView::changeIcons()
{
    const auto actions = ui->searchLine->actions();
    for (QAction* action : actions)
    {
        ui->searchLine->removeAction(action);
        delete action;
    }

    if (isDarkTheme(this)) {
        ui->searchLine->addAction(QIcon(":/icons/light/icon_search_light"), QLineEdit::LeadingPosition);
        ui->addPasswordButton->setIcon(QIcon(":/icons/light/icon_add_light"));
        ui->deletePasswordButton->setIcon(QIcon(":/icons/light/icon_delete_light"));
        ui->lockButton->setIcon(QIcon(":/icons/light/icon_lock_light"));
        ui->importButton->setIcon(QIcon(":/icons/light/icon_import_light"));
        ui->exportButton->setIcon(QIcon(":/icons/light/icon_export_light"));
        ui->settingsButton->setIcon(QIcon(":/icons/light/icon_settings_light"));
    } else {
        ui->searchLine->addAction(QIcon(":/icons/dark/icon_search_dark"), QLineEdit::LeadingPosition);
        ui->addPasswordButton->setIcon(QIcon(":/icons/dark/icon_add_dark"));
        ui->deletePasswordButton->setIcon(QIcon(":/icons/dark/icon_delete_dark"));
        ui->lockButton->setIcon(QIcon(":/icons/dark/icon_lock_dark"));
        ui->importButton->setIcon(QIcon(":/icons/dark/icon_import_dark"));
        ui->exportButton->setIcon(QIcon(":/icons/dark/icon_export_dark"));
        ui->settingsButton->setIcon(QIcon(":/icons/dark/icon_settings_dark"));
    }
}

void VaultView::onThemeChanged()
{
    changeIcons();
    emit themeChanged();
}

void VaultView::onSyncRequested()
{
    loadDataToList("");
}
