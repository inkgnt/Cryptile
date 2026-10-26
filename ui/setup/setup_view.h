#pragma once

#include <QWidget>


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
};
