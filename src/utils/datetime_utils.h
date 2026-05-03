#pragma once

#include <cstdint>
#include <QDateTime>

inline uint64_t toQDateTimeTimestamp(const QDateTime& date) {
    if (date.isNull()) return 0;
    return date.toUTC().toMSecsSinceEpoch();
}

inline QDateTime froQDateTimeTimestamp(uint64_t msecs) {
    if (msecs == 0) return QDateTime();
    return QDateTime::fromMSecsSinceEpoch(msecs, Qt::UTC);
}