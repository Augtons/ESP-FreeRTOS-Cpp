#ifndef FREERTOS_CPP_HPP
#define FREERTOS_CPP_HPP

#include <memory>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos_types.hpp"

namespace augtons {
    namespace freertos {
        template<typename ArgType = void>
        struct task_shared_data {
            bool has_deleted = false;
            TaskHandle_t task_handle = nullptr;
            FuncType_t<ArgType> function;
            ArgType args;

            explicit task_shared_data(const FuncType_t<ArgType>& function, InArgType<ArgType> args)
                : function(function)
                , args(std::forward<InArgType<ArgType>>(args)) {}
        };

        template<>
        struct task_shared_data<void> {
            bool has_deleted = false;
            TaskHandle_t task_handle = nullptr;
            FuncType_t<void> function;

            explicit task_shared_data(const FuncType_t<>& function)
                : function(function) {}
        };

        template<typename ArgType = void>
        using shared_task_data_ptr = std::shared_ptr<task_shared_data<ArgType>>;
    }

    namespace freertos {
        namespace details {
            template<typename ArgType = void>
            void delete_task_from_shared_data(task_shared_data<ArgType>* data) {
                auto handle = data->task_handle;
                data->has_deleted = true;
                data->task_handle = nullptr;
                vTaskDelete(handle);
            }

            template<typename ArgType = void>
            inline void delete_task_from_shared_data(shared_task_data_ptr<ArgType> data) {
                delete_task_from_shared_data(data.get());
            }
        }
    }
}

template<typename Arg>
class augtons::freertos::task {
    friend class task_factory<Arg>;
private:
    shared_task_data_ptr<Arg> shared_data = nullptr;

    explicit task(shared_task_data_ptr<Arg> data): shared_data(data) {}
public:
    task() = default;
    task(task&) = default;
    task(task&&) noexcept = default;
    task& operator=(const task&) = default;
    task& operator=(task&&) noexcept = default;

    task& operator=(nullptr_t) {
        this->~task();
        return *this;
    }

    ~task() {
        if (is_null()) {
            return;
        }
        if (shared_data.use_count() <= 1 && !(shared_data->has_deleted)) {
            delete_task();
        }
        shared_data = nullptr;
    }

    inline long use_count() const {
        if (is_null()) {
            return 1;
        }
        return shared_data.use_count();
    }

    inline bool is_null() const {
        return shared_data == nullptr || shared_data->task_handle == nullptr;
    }

    inline bool has_deleted() const {
        if (is_null()) {
            FreeRTOSCpp_LogW("Calling \"has_deleted()\" on a null task object always returns true.");
            return true;
        }
        return shared_data->has_deleted;
    }

    TaskHandle_t native_handle_not_null() const {
        if (is_null()) {
            FreeRTOSCpp_LogE("Calling \"native_handle()\" on a null task object.");
            FreeRTOSCpp_LogE("You can use \"native_handle()\" instead.");
            abort();
        }
        if (shared_data->task_handle == nullptr) {
            FreeRTOSCpp_LogE("This task object contains a null task handle. This might be due to the library itself, you can submit an issue for this problem.");
            abort();
        }
        return shared_data->task_handle;
    }

    TaskHandle_t native_handle() const {
        if (is_null()) {
            return nullptr;
        }
        return shared_data->task_handle;
    }

    inline explicit operator TaskHandle_t() const {
        return native_handle();
    }

    bool operator==(const task& other) const {
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

    void delete_task() {
        if (is_null()) {
            FreeRTOSCpp_LogW("Try to delete a task from a task object that is null.");
            return;
        }
        if (has_deleted()) {
            shared_data = nullptr;
            return;
        }

        auto data = shared_data;
        shared_data = nullptr;
        details::delete_task_from_shared_data(data);
    }
};

#endif