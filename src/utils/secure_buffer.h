#pragma once

#include <sodium.h>
#include <cstdlib>
#include <stdexcept>
#include<cstdint>
class SecureBuffer
{
public:
    SecureBuffer(size_t size) : size_(size)
    {
        data_ = (uint8_t*)sodium_malloc(size_);
        if (!data_)
            throw std::bad_alloc();

        if(sodium_mlock(data_, size_) == -1) {
            sodium_free(data_);
            data_ = nullptr;
            size_ = 0;
            throw std::runtime_error("sodium_mlock failure");
        }
    }

    ~SecureBuffer() {
        if(data_)
            sodium_free(data_);
        size_ = 0;
    }

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

private:
    uint8_t* data_ = nullptr;
    size_t size_ = 0;
};
