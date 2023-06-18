#ifndef FREERTOS_CPP_TASK_FACTORY_HPP
#define FREERTOS_CPP_TASK_FACTORY_HPP

#include "freertos.hpp"
#include "cstring"

template<typename ArgType>
class augtons::freertos::task_factory {
    using Func = FuncType_t<ArgType>;
public:

    static void task_fun(void* _arg) {
        auto *data = (task_shared_data<ArgType>*)(_arg);
        if (data == nullptr) {
            FreeRTOSCpp_LogE("Unexpected situation: the argument received by the native task function is NULL.");
            abort();
        }
        data->function(std::forward<ArgType>(data->args));
        details::delete_task_from_shared_data(data);
    }

    static auto create(
        const char *const name,
        const uint32_t stack_size,
        const UBaseType_t priority,
        InArgType<ArgType> task_args,
        const Func& func,
        BaseType_t core_id = tskNO_AFFINITY
    ) -> task<ArgType> {

        auto data = std::make_shared<task_shared_data<ArgType>>(func, std::forward<InArgType<ArgType>>(task_args));
        auto ret = task<ArgType>(data);

        if (xTaskCreatePinnedToCore(task_fun, name, stack_size, ret.shared_data.get(),
                        priority, &ret.shared_data->task_handle, core_id) != pdPASS) {
            ret = nullptr;
        }

        return ret;
    }
};

template<>
class augtons::freertos::task_factory<void> {
    using Func = FuncType_t<void>;

public:

    static void task_fun(void* _arg) {
        auto *data = (task_shared_data<>*)(_arg);
        if (data == nullptr) {
            FreeRTOSCpp_LogE("Unexpected situation: the argument received by the native task function is NULL.");
            abort();
        }
        data->function();
        details::delete_task_from_shared_data(data);
    }

    static auto create(
        const char *const name,
        const uint32_t stack_size,
        const UBaseType_t priority,
        const Func& func,
        BaseType_t core_id = tskNO_AFFINITY
    ) -> task<> {
        auto data = std::make_shared<task_shared_data<>>(func);
        auto ret = task<>(data);
        if (xTaskCreatePinnedToCore(task_fun, name, stack_size, ret.shared_data.get(),
                                    priority, &ret.shared_data->task_handle, core_id) != pdPASS) {
            ret = nullptr;
        }
        return ret;
    }
};

template<>
class augtons::freertos::task_builder<void> {
    using Func = FuncType_t<void>;
private:
    char m_name[CONFIG_FREERTOS_MAX_TASK_NAME_LEN + 1] = {0};
    uint32_t m_stack_size_num = 0;
    UBaseType_t m_priority = 0;
    BaseType_t m_core_id = tskNO_AFFINITY;
public:
    explicit task_builder(const char *name) {
        strncpy(m_name, name, sizeof(m_name));
    }

    task_builder& stack(uint32_t size) {
        this->m_stack_size_num = size;
        return *this;
    }

    task_builder& priority(UBaseType_t p) {
        this->m_priority = p;
        return *this;
    }

    task_builder& core_id(BaseType_t core) {
        m_core_id = core;
        return *this;
    }

    task<> bind(const Func& func) {
        return task_factory<>::create(m_name, m_stack_size_num, m_priority, func, m_core_id);
    }
};

template<typename ArgType>
class augtons::freertos::task_builder {
    using Func = FuncType_t<ArgType>;
    char m_name[CONFIG_FREERTOS_MAX_TASK_NAME_LEN + 1] = {0};
    uint32_t m_stack_size_num = 0;
    UBaseType_t m_priority = 0;
    BaseType_t m_core_id = tskNO_AFFINITY;
public:
    explicit task_builder(const char *name) {
        strncpy(m_name, name, sizeof(m_name));
    }

    task_builder& stack(uint32_t size) {
        this->m_stack_size_num = size;
        return *this;
    }

    task_builder& priority(UBaseType_t p) {
        this->m_priority = p;
        return *this;
    }

    task_builder& core_id(BaseType_t core) {
        m_core_id = core;
        return *this;
    }

    task<ArgType> bind(InArgType<ArgType> arg, const Func& func) {
        return task_factory<ArgType>::create(m_name, m_stack_size_num, m_priority,
                                             std::forward<InArgType<ArgType>>(arg), func, m_core_id);
    }
};


#endif