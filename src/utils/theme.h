#pragma once

#include <QGuiApplication>
#include <QIcon>
#include <QIconEngine>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QString>
#include <QStyleHints>

class ThemedIconEngine final : public QIconEngine
{
public:
    explicit ThemedIconEngine(const QString& offName, const QString& onName = QString())
        : m_offName(offName)
        , m_onName(onName)
        , m_darkTheme(load(":/icons/light/", m_offName, m_onName))
        , m_lightTheme(load(":/icons/dark/", m_offName, m_onName))
    {}

    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override
    {
        current().paint(painter, rect, Qt::AlignCenter, mode, state);
    }

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        return current().pixmap(size, mode, state);
    }

    QPixmap scaledPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
        return current().pixmap(size, scale, mode, state);
    }

    QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) override
    {
        return current().actualSize(size, mode, state);
    }

    QIconEngine* clone() const override
    {
        return new ThemedIconEngine(*this);
    }

private:
    static QIcon load(const QString& path, const QString& offName, const QString& onName)
    {
        QIcon icon;
        icon.addFile(path + offName, QSize(), QIcon::Normal, QIcon::Off);
        if (!onName.isEmpty())
            icon.addFile(path + onName, QSize(), QIcon::Normal, QIcon::On);

        return icon;
    }

    static bool isDark() {
        const Qt::ColorScheme scheme = QGuiApplication::styleHints()->colorScheme();

        if (scheme!=Qt::ColorScheme::Unknown)
            return scheme == Qt::ColorScheme::Dark;

        return QGuiApplication::palette().color(QPalette::Window).lightness() < 128;
    }

    const QIcon& current() const { return isDark() ? m_darkTheme : m_lightTheme; }

    QString m_offName;
    QString m_onName;
    QIcon m_darkTheme;
    QIcon m_lightTheme;
};
