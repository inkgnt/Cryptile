#pragma once

#include <cstdint>
#include <QDateTime>
#include <QTimeZone>

inline uint64_t toQDateTimeTimestamp(const QDateTime& date) {
    if (date.isValid()) return 0;
    return date.toUTC().toMSecsSinceEpoch();
}

inline QDateTime fromQDateTimeTimestamp(uint64_t msecs) {
    if (msecs == 0) return {};
    return QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(msecs), QTimeZone::UTC);
}