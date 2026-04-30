#pragma once

#include <vector>
#include <QString>
#include <QFile>

static constexpr uint64_t SALT_LENGTH = 16;
static constexpr uint64_t HASH_LENGTH = 32;
static constexpr uint64_t ITERATIONS = 10000;

static constexpr uint64_t SCRYPT_N = 32768;           // Iteration count
static constexpr uint64_t SCRYPT_r = 32;              // Block size
static constexpr uint64_t SCRYPT_p = 2;               // Parallelization factor
static constexpr uint64_t SCRYPT_MAXMEM = 268435456;  // 0 = no restrictions, 1048576 - 1 mb

const QString HASH_FILE_PATH = "pHash.dat";

inline bool isHashFileExists() {
    QFile file(HASH_FILE_PATH);
    return file.exists();
}

std::vector<uint8_t> encryptAES256(const std::vector<uint8_t>& plaintext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);
std::vector<uint8_t> decryptAES256(const std::vector<uint8_t>& ciphertext, const std::vector<uint8_t>& key, const std::vector<uint8_t>& iv);

bool loadHashAndSaltFromFile(std::vector<uint8_t> &storedSalt, std::vector<uint8_t> &storedHash);
void saveHashAndSaltToFile(const std::vector<uint8_t> &Salt, const std::vector<uint8_t> &Hash);

std::vector<uint8_t> generatePBKDF2Hash(const QString &password, const std::vector<uint8_t> &salt);
std::vector<uint8_t> generateScryptKey(const QString &password, const std::vector<uint8_t> &salt);

std::vector<uint8_t> generateSalt();
std::vector<uint8_t> generateIV();
