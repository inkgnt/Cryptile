#pragma once

#include <QWidget>
#include <QToolButton>


namespace Ui {
    class SetupView;
}

class SetupView : public QWidget
{
    Q_OBJECT

public:
    explicit SetupView(QWidget *parent = nullptr);
    ~SetupView();

signals:
    void registerSuccess();

private slots:
    void onsubmitButtonClicked();

private:
    Ui::SetupView *ui;

    bool isFirstPasswordVisible = false;
    bool isSecondPasswordVisible = false;
    QToolButton *toggleButton1 = nullptr;
    QToolButton *toggleButton2 = nullptr;
};
