#pragma once

#include "lock_limit.h"

#include <sodium.h>

#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include <QDebug>

class SecureBuffer
{
public:
    SecureBuffer() = default;

    SecureBuffer(size_t size) : size_(size)
    {
        data_ = (uint8_t*)sodium_malloc(size_);
        qDebug() << "sodium_malloc triggered";
        if (!data_) {
            qDebug() << "sodium_malloc failure";
            throw std::bad_alloc();
        }

        if(sodium_mlock(data_, size_) == -1) {
            qDebug() << "sodium_mlock failure";
#ifdef _WIN32
            if(!increaseWindowsLockLimit(size_) || sodium_mlock(data_, size_) == -1) {
                qDebug() << "increaseWindowsLockLimit failure";
                sodium_free(data_);
                data_ = nullptr;
                size_ = 0;
                throw std::runtime_error("Critical failure! Operating System refused to lock memory pages.");
            }
#endif
        }
    };

    ~SecureBuffer() {
        if(data_) {
            qDebug() << "sodium_free triggered";
            sodium_free(data_);
        }
        size_ = 0;
    };

    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    SecureBuffer(SecureBuffer&& other) noexcept
        :data_(other.data_), size_(other.size_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
    };

    SecureBuffer& operator=(SecureBuffer&& other) noexcept
    {
        if(this != &other) {
        if(data_)
            sodium_free(data_);

        data_ = other.data_;
        size_ = other.size_;

        other.data_ = nullptr;
        other.size_ = 0;

        }
        return *this;
    };

    uint8_t* data() noexcept { return data_; };
    const uint8_t* data() const noexcept { return data_; };
    size_t size() const noexcept { return size_; };
    bool empty() const noexcept {return !size_;  };

private:
    uint8_t* data_ = nullptr;
    size_t size_ = 0;
};
