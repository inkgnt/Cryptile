#pragma once

#include "lock_limit.h"

#include <sodium.h>

#include <cstdlib>
#include <stdexcept>
#include <cstdint>
#include <QDebug>
#include <algorithm>
#include <cstring>


class SecureBuffer
{
public:

    SecureBuffer() = default;

    explicit SecureBuffer(std::size_t size) {
        if (size > 0) {
            reallocate(size);
            size_ = size;
        }
    }

    ~SecureBuffer() {
        destroy();
    }

    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;

    SecureBuffer(SecureBuffer&& other) noexcept
        :data_(other.data_), size_(other.size_), capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    };

    SecureBuffer& operator=(SecureBuffer&& other) noexcept
    {
        if(this != &other) {
            destroy();

            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;

            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    std::uint8_t* data() noexcept { return data_; };
    const std::uint8_t* data() const noexcept { return data_; };
    std::size_t size() const noexcept { return size_; };
    std::size_t capacity() const noexcept { return capacity_; };
    bool empty() const noexcept {return !size_;  };

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_)
            return;
        reallocate(new_capacity);
    }

    void resize(std::size_t new_size) {

        if (new_size > capacity_) {
            std::size_t new_capacity = std::max(new_size, capacity_ + capacity_ / 2);
            reserve(new_capacity);
        }
        else if (new_size < size_) {
            sodium_memzero(data_ + new_size, size_ - new_size);
        }
        size_ = new_size;
    }


private:
    inline static std::atomic<std::size_t> global_memory_used{0};
    inline static std::atomic<std::size_t> global_counter_free{0};
    inline static std::atomic<std::size_t> global_counter_malloc{0};

    std::uint8_t* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    void destroy() {
        if (data_) {
            qDebug() << "sodium_free triggered" <<  global_counter_free++;
            sodium_free(data_);

            global_memory_used -= capacity_;
            qDebug() << "total memory used after sodium_free:" << global_memory_used ;
        }
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
    }

    void reallocate(std::size_t new_capacity) {
        std::uint8_t* new_data = (std::uint8_t*)sodium_malloc(new_capacity);
        qDebug() << "sodium_malloc" << global_counter_malloc++ << "triggered, size:" << new_capacity;

        global_memory_used += new_capacity;
        qDebug() << "total memory used after sodium_malloc:" << global_memory_used ;

        if (!new_data) {
            throw std::bad_alloc();
        }

        sodium_memzero(new_data, new_capacity);

        if (sodium_mlock(new_data, new_capacity) == -1) {
#ifdef _WIN32
            if (!increaseWindowsLockLimit(new_capacity) || sodium_mlock(new_data, new_capacity) == -1) {
                sodium_free(new_data);
                throw std::runtime_error("Critical failure! Operating System refused to lock memory pages.");
            }
#else
            sodium_free(new_data);
            throw std::runtime_error("Critical failure! Operating System refused to lock memory pages.");
#endif
        }

        if (data_) {
            std::size_t copy_len = std::min(size_, new_capacity);
            if (copy_len > 0) {
                std::memcpy(new_data, data_, copy_len);
            }

            sodium_free(data_);

            qDebug() << "sodium_free" << global_counter_free++ << "triggered, size:" << new_capacity;

            global_memory_used -= capacity_;
            qDebug() << "total memory used after sodium_free:" << global_memory_used ;
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

};
