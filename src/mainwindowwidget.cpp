#include "mainwindowwidget.h"
#include "ui_mainwindowwidget.h"

#include "passwordcardwidget.h"
#include "dialog.h"
#include "passwordform.h"

#include "databasemanager.h"

inline bool isDarkTheme(QWidget* w) {
    return w->palette().color(QPalette::Window).lightness() < 128;
}

MainWindowWidget::MainWindowWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindowWidget)
{
    ui->setupUi(this);

    changeIcons();

    loadDataToList("");

    connect(ui->SEARCHline, &QLineEdit::textChanged, this, &MainWindowWidget::onSEARCHlineTextChanged);

    connect(ui->ADDPSWDbtn, &QPushButton::clicked, this, &MainWindowWidget::onADDPSWDbtnClicked);
    connect(ui->DELPSWDbtn, &QPushButton::clicked, this, &MainWindowWidget::onDELPSWDbtnClicked);
    connect(ui->LOCKbtn, &QPushButton::clicked, this, &MainWindowWidget::onLOCKbtnClicked);
    connect(ui->IMPORTbtn, &QPushButton::clicked, this, &MainWindowWidget::onIMPORTbtnClicked);
    connect(ui->EXPORTbtn, &QPushButton::clicked, this, &MainWindowWidget::onEXPORTbtnClicked);
    connect(ui->SETTINGSbtn, &QPushButton::clicked, this, &MainWindowWidget::onSETTINGSbtnClicked);
}

MainWindowWidget::~MainWindowWidget()
{
    DatabaseManager::instance().closeDatabase();
    delete ui;
}

void MainWindowWidget::onSEARCHlineTextChanged(const QString &filter)
{
    loadDataToList(filter);
}

void MainWindowWidget::onADDPSWDbtnClicked()
{
    auto pw = new PasswordForm;
    auto d = new Dialog(pw, this);
    d->exec();

    loadDataToList("");
}

void MainWindowWidget::onDELPSWDbtnClicked()
{
    QListWidgetItem *selectedItem = ui->LISTwidget->currentItem();
    if (!selectedItem) return;

    int id = selectedItem->data(Qt::UserRole).toInt();

    if (DatabaseManager::instance().deleteRecord(id)) {
        int row = ui->LISTwidget->row(selectedItem);
        QListWidgetItem *item = ui->LISTwidget->takeItem(row);
        delete item;
    }

    loadDataToList("");
}

void MainWindowWidget::onLOCKbtnClicked()
{
    emit lockRequested();
}

void MainWindowWidget::onIMPORTbtnClicked()
{
    //
}

void MainWindowWidget::onEXPORTbtnClicked()
{
    //
}

void MainWindowWidget::onSETTINGSbtnClicked()
{
    //
}

void MainWindowWidget::loadDataToList(const QString &filter)
{
    ui->LISTwidget->clear();
    QList<PasswordRecord> records = DatabaseManager::instance().getAllRecords();

    for (PasswordRecord &record : records)
    {
        if (!(filter.isEmpty() || record.url.contains(filter, Qt::CaseInsensitive)))
            continue;

        auto *widget = new PasswordCardWidget(record, this);

        connect(widget, &PasswordCardWidget::syncRequested, this, &MainWindowWidget::onSyncRequested);
        connect(this, &MainWindowWidget::themeChanged, widget, &PasswordCardWidget::onThemeChanged);

        auto *item = new QListWidgetItem(ui->LISTwidget);
        item->setData(Qt::UserRole, record.id);
        item->setSizeHint(widget->sizeHint());

        ui->LISTwidget->addItem(item);
        ui->LISTwidget->setItemWidget(item, widget);
    }
}

void MainWindowWidget::changeIcons()
{
    const auto actions = ui->SEARCHline->actions();
    for (QAction* action : actions)
    {
        ui->SEARCHline->removeAction(action);
        delete action;
    }

    if (isDarkTheme(this)) {
        ui->SEARCHline->addAction(QIcon(":/icons/light/icon_search_light"), QLineEdit::LeadingPosition);
        ui->ADDPSWDbtn->setIcon(QIcon(":/icons/light/icon_add_light"));
        ui->DELPSWDbtn->setIcon(QIcon(":/icons/light/icon_delete_light"));
        ui->LOCKbtn->setIcon(QIcon(":/icons/light/icon_lock_light"));
        ui->IMPORTbtn->setIcon(QIcon(":/icons/light/icon_import_light"));
        ui->EXPORTbtn->setIcon(QIcon(":/icons/light/icon_export_light"));
        ui->SETTINGSbtn->setIcon(QIcon(":/icons/light/icon_settings_light"));
    } else {
        ui->SEARCHline->addAction(QIcon(":/icons/dark/icon_search_dark"), QLineEdit::LeadingPosition);
        ui->ADDPSWDbtn->setIcon(QIcon(":/icons/dark/icon_add_dark"));
        ui->DELPSWDbtn->setIcon(QIcon(":/icons/dark/icon_delete_dark"));
        ui->LOCKbtn->setIcon(QIcon(":/icons/dark/icon_lock_dark"));
        ui->IMPORTbtn->setIcon(QIcon(":/icons/dark/icon_import_dark"));
        ui->EXPORTbtn->setIcon(QIcon(":/icons/dark/icon_export_dark"));
        ui->SETTINGSbtn->setIcon(QIcon(":/icons/dark/icon_settings_dark"));
    }
}

void MainWindowWidget::onThemeChanged()
{
    changeIcons();
    emit themeChanged();
}

void MainWindowWidget::onSyncRequested()
{
    loadDataToList("");
}
