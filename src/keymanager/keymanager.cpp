#include "keymanager.h"
#include <sodium.h>

KeyManager& KeyManager::instance()
{
    static KeyManager instance;
    return instance;
}

KeyManager::KeyManager(QObject* parent)
    : QObject(parent), key(KEY_SIZE)
{
    sessionCheckTimer = new QTimer(this);
    connect(sessionCheckTimer, &QTimer::timeout, this, &KeyManager::checkSessionValidity);
    sessionCheckTimer->start(SESSION_TIMEOUT_MS);
}

KeyManager::~KeyManager()
{
    delete sessionCheckTimer;
    sessionCheckTimer = nullptr;
    clearKey();
}

void KeyManager::setKey(const std::vector<uint8_t>& newKey)
{
    std::lock_guard<std::mutex> lock(mtx);

    std::fill(key.begin(), key.end(), 0);

    std::copy(newKey.begin(), newKey.end(), key.begin());

    initialized = true;
    updateLastActivity();
}

std::vector<uint8_t> KeyManager::getKey()
{
    std::lock_guard<std::mutex> lock(mtx);

    if(!initialized)
    {
        clearKey();
        return{};
    }

    if(!isSessionValid())
    {
        clearKey();
        return {};
    }

    updateLastActivity();
    return key;
}

void KeyManager::clearKey()
{
    std::lock_guard<std::mutex> lock(mtx);

    if (initialized) {
        sodium_memzero(key.data(), key.size());
        initialized = false;

        emit keyCleared();
    }
}



bool KeyManager::isSessionValid() const
{
    if (initialized)
        return lastActivity.msecsTo(QDateTime::currentDateTime()) < SESSION_TIMEOUT_MS;

    return false;
}

void KeyManager::updateLastActivity()
{
    lastActivity = QDateTime::currentDateTime();
}

void KeyManager::checkSessionValidity()
{
    std::lock_guard<std::mutex> lock(mtx);

    if (initialized && lastActivity.msecsTo(QDateTime::currentDateTime()) >= SESSION_TIMEOUT_MS)
        clearKey();

}
