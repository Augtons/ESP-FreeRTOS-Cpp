//
// Created by augtons on 23-6-16.
//

#ifndef FREERTOS_CPP_QUEUE_HPP
#define FREERTOS_CPP_QUEUE_HPP

#include <optional>
#include "freertos.hpp"
#include "freertos/queue.h"

namespace augtons {
    namespace freertos {
        namespace details {
            struct queue_shared_data {
                bool has_deleted = false;
                QueueHandle_t handle = nullptr;
            };
        }

        using queue_shared_data_ptr = std::shared_ptr<details::queue_shared_data>;
    }
}

template<typename T>
class augtons::freertos::queue {
    using PointerType = typename std::add_pointer<typename std::remove_reference<T>::type>::type;
    static_assert(!std::is_reference<T>::value, "Don't support reference type.");
private:
    queue_shared_data_ptr shared_data = nullptr;
public:

    queue() = default;

    explicit queue(size_t length) {
        shared_data = std::make_shared<details::queue_shared_data>();
        shared_data->handle = xQueueCreate(sizeof(PointerType), length); // 用指针，记得特化引用
    }

    queue(queue&) = default;
    queue(queue&&) noexcept = default;
    queue& operator=(const queue&) = default;
    queue& operator=(queue&&) noexcept = default;

    queue& operator=(nullptr_t) {
        this->~queue();
        return *this;
    }

    ~queue() {
        if (is_null()) {
            return;
        }
        if (shared_data.use_count() <= 1 && !(shared_data->has_deleted)) {
            delete_queue();
        }
        shared_data = nullptr;
    }

    inline bool is_null() const {
        return shared_data == nullptr || shared_data->handle == nullptr;
    }

    inline bool has_deleted() const {
        if (is_null()) {
            FreeRTOSCpp_LogW("Calling \"has_deleted()\" on a null queue object always returns true.");
            return true;
        }
        return shared_data->has_deleted;
    }

    void delete_queue() {
        if (is_null()) {
            FreeRTOSCpp_LogW("Try to delete a queue from a queue object that is null.");
            return;
        }
        if (has_deleted()) {
            shared_data = nullptr;
            return;
        }

        vQueueDelete(shared_data->handle);
        shared_data->handle = nullptr;
        shared_data->has_deleted = true;
        shared_data = nullptr;
    }

    inline long use_count() const {
        if (is_null()) {
            return 1;
        }
        return shared_data.use_count();
    }

    QueueHandle_t native_handle_not_null() const {
        if (is_null()) {
            FreeRTOSCpp_LogE("Calling \"native_handle()\" on a null queue object.");
            FreeRTOSCpp_LogE("You can use \"native_handle()\" instead.");
            abort();
        }
        if (shared_data->handle == nullptr) {
            FreeRTOSCpp_LogE("This queue object contains a null queue handle. This might be due to the library itself, you can submit an issue for this problem.");
            abort();
        }
        return shared_data->handle;
    }

    QueueHandle_t native_handle() const {
        if (is_null()) {
            return nullptr;
        }
        return shared_data->handle;
    }

    inline explicit operator QueueHandle_t() const {
        return native_handle();
    }

    bool operator==(const queue& other) const {
        if (this == &other) {
            return true;
        }
        if (this->is_null() && other.is_null()) {
            return true;
        }
        if (!this->is_null() && !other.is_null() && this->native_handle() == other.native_handle()) {
            return true;
        }
        return false;
    }

    BaseType_t send(T&& data, TickType_t timeout = portMAX_DELAY) const {
        if (is_null() || has_deleted()) {
            return pdFAIL;
        }
        T* new_data = new T(std::move(data));    // 重新new一次，通过移动右值来延长生命周期(C+17前)
                                                 // 重新new一次，将临时量实质化(C++17起)用于传入队列
        assert(new_data);
        return xQueueSend(shared_data->handle, &new_data, timeout);
    }

    BaseType_t send(T& data, TickType_t timeout = portMAX_DELAY) const { // 不要加const
        if (is_null() || has_deleted()) {
            return pdFAIL;
        }
        auto *new_data = new T(data);    // 重新new保证正确拷贝
        assert(new_data);
        return xQueueSend(shared_data->handle, &new_data, timeout);
    }

    bool receive_to(T& out, TickType_t timeout = portMAX_DELAY) const {
        if (is_null() || has_deleted()) {
            return false;
        }
        T* new_data = nullptr;
        if (xQueueReceive(shared_data->handle, &new_data, timeout) == pdTRUE) {
            assert(new_data);
            out = std::move(*new_data);
            delete new_data;
            return true;
        } else {
            return false;
        }
    }

#if __cplusplus >= 201703L
    std::optional<T> receive(TickType_t timeout = portMAX_DELAY) const {
        if (is_null() || has_deleted()) {
            return std::nullopt;
        }
        T* new_data = nullptr;
        if (xQueueReceive(shared_data->handle, &new_data, timeout) == pdTRUE) {
            assert(new_data);
            T out = std::move(*new_data);
            delete new_data;
            return out;
        } else {
            return std::nullopt;
        }
    }
#endif
};

#endif //FREERTOS_CPP_QUEUE_HPP
