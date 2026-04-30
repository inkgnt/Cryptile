#pragma once

#include <QWidget>

namespace Ui {
class MainWindowWidget;
}

class MainWindowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindowWidget(QWidget *parent = nullptr);
    ~MainWindowWidget();

signals:
    void lockRequested();
    void themeChanged();

public slots:
    void onThemeChanged();

private slots:
    void onSEARCHlineTextChanged(const QString &arg1);

    void onADDPSWDbtnClicked();
    void onDELPSWDbtnClicked();
    void onLOCKbtnClicked();
    void onIMPORTbtnClicked();
    void onEXPORTbtnClicked();
    void onSETTINGSbtnClicked();

    void onSyncRequested();

private:
    Ui::MainWindowWidget *ui;

    void loadDataToList(const QString& filter);

    void changeIcons();
};
