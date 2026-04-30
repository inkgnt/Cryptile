#pragma once

#include <QWidget>

namespace Ui {
class RegisterWidget;
}

class RegisterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWidget(QWidget *parent = nullptr);
    ~RegisterWidget();

signals:
    void registerSuccess();

public slots:
    void onThemeChanged();

private slots:
    void onSUBMITbtnClicked();

private:
    Ui::RegisterWidget *ui;

    bool isFirstPasswordVisible = false;
    bool isSecondPasswordVisible = false;
    QToolButton *toggleButton1 = nullptr;
    QToolButton *toggleButton2 = nullptr;
};

