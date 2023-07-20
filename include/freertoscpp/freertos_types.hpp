#ifndef FREERTOS_CPP_TYPES_HPP
#define FREERTOS_CPP_TYPES_HPP

#include <functional>
#include "esp_log.h"

#define FreeRTOSCpp_LogI(FORMAT, ...) \
    ESP_LOGI("FreeRTOS-Cpp", FORMAT, ##__VA_ARGS__)

#define FreeRTOSCpp_LogD(FORMAT, ...) \
    ESP_LOGD("FreeRTOS-Cpp", FORMAT, ##__VA_ARGS__)

#define FreeRTOSCpp_LogW(FORMAT, ...) \
    ESP_LOGW("FreeRTOS-Cpp", FORMAT, ##__VA_ARGS__)

#define FreeRTOSCpp_LogE(FORMAT, ...) \
    ESP_LOGE("FreeRTOS-Cpp", FORMAT, ##__VA_ARGS__)

namespace augtons
{
    namespace freertos {
        template<typename Arg = void>
        class task;

        template<typename ArgType = void>
        class task_factory;

        template<typename ArgType = void>
        class task_builder;

    }

    namespace freertos {
        template<typename ArgType>
        void task_fun(void* _arg);

        template<>
        void task_fun<void>(void* _arg);
    }

    namespace freertos {
        template<typename ArgType>
        struct FuncType {
            using type = std::function<void(ArgType)>;
        };

        template<>
        struct FuncType<void> {
            using type = std::function<void()>;
        };

        template<typename ArgType = void>
        using FuncType_t = typename FuncType<ArgType>::type;
    }

    namespace freertos {
        template<typename ArgType>
        using InArgType = typename std::conditional<
                std::is_reference<ArgType>::value,
                ArgType,
                const ArgType&
            >::type;
    }
}

namespace augtons {
    namespace freertos {
        template<typename T>
        class queue;
    }
}

#endif
