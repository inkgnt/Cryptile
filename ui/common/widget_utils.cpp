#include "widget_utils.h"
#include "utils/theme.h"

#include <QToolButton>
#include <QWidgetAction>


void addVisibilityToggle(QLineEdit* edit)
{
    auto* button = new QToolButton;
    button->setCursor(Qt::ArrowCursor);
    button->setCheckable(true);
    button->setIcon(QIcon(new ThemedIconEngine("show", "hide")));
    button->setStyleSheet(R"(
        QToolButton {
            border: none;
            background-color: transparent;
        }
        QToolButton:hover {
            background-color: rgba(150, 150, 150, 50);
            border-radius: 4px;
        }
    )");

    auto* action = new QWidgetAction(edit);
    action->setDefaultWidget(button);
    edit->addAction(action, QLineEdit::TrailingPosition);

    QObject::connect(button, &QToolButton::toggled, edit, [edit](bool visible) {
        edit->setEchoMode(visible ? QLineEdit::Normal : QLineEdit::Password);
    });
}
