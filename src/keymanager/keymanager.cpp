#include "keymanager.h"

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

    clearKeyPRNG();
}

void KeyManager::clearKeyPRNG()
{
    if (initialized) {
        std::generate(key.begin(), key.end(), []() {
            return static_cast<uint8_t>(QRandomGenerator::system()->generate() & 0xFF);
        });
        std::fill(key.begin(), key.end(), 0);
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
        clearKeyPRNG();

}
