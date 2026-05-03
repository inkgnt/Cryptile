#pragma once

#include <QDateTime>
#include <QObject>
#include <QTimer>
#include <mutex>
#include <QRandomGenerator>

class KeyManager : public QObject
{
    Q_OBJECT
public:
    static constexpr uint64_t KEY_SIZE = 32;
    static constexpr uint64_t SESSION_TIMEOUT_MS = 300000; // 5 min, 10000 - 1 sec, 60000 - 1 min

    KeyManager(const KeyManager&) = delete;
    KeyManager& operator=(const KeyManager&) = delete;

    static KeyManager& instance();

    void setKey(const std::vector<uint8_t>& newKey);
    std::vector<uint8_t> getKey();

    void clearKey();

    bool isSessionValid() const;
    void updateLastActivity();

signals:
    void keyCleared();

private:
    explicit KeyManager(QObject* parent = nullptr);
    ~KeyManager();

    void checkSessionValidity();
    void clearKeyPRNG();

    mutable std::mutex mtx;
    std::vector<uint8_t> key;
    QDateTime lastActivity;
    bool initialized = false;

    QTimer* sessionCheckTimer = nullptr;
};
