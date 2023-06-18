#ifndef FREERTOS_CPP_SEMPHR_HPP
#define FREERTOS_CPP_SEMPHR_HPP

#include "freertos.hpp"
#include "freertos/semphr.h"

namespace augtons {
    namespace freertos {
        class recurse_mutex;
        class generic_mutex;
        class binary_semphr;
        class counting_semphr;

        template<typename Mutex>
        class mutex_locker;
    }
}

template<typename Mutex>
class augtons::freertos::mutex_locker {
private:
    Mutex& mutex;

public:
    explicit mutex_locker(Mutex& mutex) : mutex(mutex) {
        mutex.lock();
    }

    // 删除拷贝，删除所有赋值（防止多引用）
    mutex_locker(mutex_locker&) = delete;
    mutex_locker &operator=(mutex_locker&) = delete;
    mutex_locker &operator=(mutex_locker&&) noexcept = delete;
    mutex_locker(mutex_locker&&) noexcept = delete;

    ~mutex_locker() {
        mutex.unlock();
    }
};

#define __MutexDeclare(_ClassName, _Create, _Take, _Give)       \
class augtons::freertos:: _ClassName {                          \
private:                                                        \
    SemaphoreHandle_t mutex = nullptr;                          \
public:                                                         \
    _ClassName() {                                              \
        mutex = _Create();                                      \
    }                                                           \
                                                                \
    /* Disable Copy */                                          \
    _ClassName(_ClassName&) = delete;                           \
    _ClassName& operator=(_ClassName&) = delete;                \
                                                                \
    /* Enable Move*/                                            \
    _ClassName(_ClassName&& other) noexcept {                   \
        if (this == &other) {                                   \
            return;                                             \
        }                                                       \
        mutex = other.mutex;                                    \
        other.mutex = nullptr;                                  \
    };                                                          \
    _ClassName& operator=(_ClassName&& other) noexcept {        \
        if (this == &other) {                                   \
            return *this;                                       \
        }                                                       \
        mutex = other.mutex;                                    \
        other.mutex = nullptr;                                  \
        return *this;                                           \
    };                                                          \
                                                                \
    ~_ClassName() {                                             \
        if (mutex != nullptr) {                                 \
           vSemaphoreDelete(mutex);                             \
        }                                                       \
    }                                                           \
                                                                \
    bool lock(TickType_t timeout = portMAX_DELAY) {             \
        return _Take(mutex, timeout);                           \
    }                                                           \
                                                                \
    void unlock() {                                             \
        _Give(mutex);                                           \
    }                                                           \
                                                                \
    inline SemaphoreHandle_t native_handle() const {            \
        return mutex;                                           \
    }                                                           \
                                                                \
    inline explicit operator SemaphoreHandle_t() const {        \
        return mutex;                                           \
    }                                                           \
};

__MutexDeclare(recurse_mutex, xSemaphoreCreateRecursiveMutex, xSemaphoreTakeRecursive, xSemaphoreGiveRecursive)
__MutexDeclare(generic_mutex, xSemaphoreCreateMutex, xSemaphoreTake, xSemaphoreGive)
__MutexDeclare(binary_semphr, xSemaphoreCreateBinary, xSemaphoreTake, xSemaphoreGive)

#undef __MutexDeclare

class augtons::freertos::counting_semphr {
private:
    SemaphoreHandle_t mutex = nullptr;
public:
    counting_semphr(UBaseType_t uxMaxCount, UBaseType_t uxInitialCount) {
        mutex = xSemaphoreCreateCounting(uxMaxCount, uxInitialCount);
    }

    /* Disable Copy */
    counting_semphr(counting_semphr&) = delete;
    counting_semphr& operator=(counting_semphr&) = delete;

    /* Enable Move */
    counting_semphr(counting_semphr&& other) noexcept {
        if (this == &other) {
            return;
        }
        mutex = other.mutex;
        other.mutex = nullptr;
    }
    counting_semphr &operator=(counting_semphr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        mutex = other.mutex;
        other.mutex = nullptr;
        return *this;
    }

    ~counting_semphr() {
        if (mutex != nullptr) {
            vSemaphoreDelete(mutex);
        }
    }

    bool lock(TickType_t timeout = portMAX_DELAY) {
        return xSemaphoreTake(mutex, timeout);
    }

    void unlock() {
        xSemaphoreGive(mutex);
    }

    inline SemaphoreHandle_t native_handle() const {
        return mutex;
    }

    inline explicit operator SemaphoreHandle_t() const {
        return mutex;
    }
};

#endif //FREERTOS_CPP_SEMPHR_HPP
