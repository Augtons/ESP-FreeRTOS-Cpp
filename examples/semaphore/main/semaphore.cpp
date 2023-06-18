#include <iostream>
#include "esp_log.h"
#include "freertoscpp/freertos.hpp"
#include "freertoscpp/freertos_task_factory.hpp"
#include "freertoscpp/semphr.hpp"

using namespace std;
using augtons::freertos::task;
using augtons::freertos::task_factory;
using augtons::freertos::task_builder;

using augtons::freertos::recurse_mutex;
using augtons::freertos::mutex_locker;


extern "C" void app_main()
{
    auto semaphore = recurse_mutex();

    auto task1 = task_builder<>("hello")
        .stack(2048)
        .priority(0)
        .bind([&] {
            mutex_locker locker(semaphore);
            ESP_LOGI("TAG", "Locked in task hello");
            vTaskDelay(pdMS_TO_TICKS(2000));
            ESP_LOGI("TAG", "Unocked in task hello");
        });

    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI("TAG", "Acquiring in main task");
    semaphore.lock(portMAX_DELAY);
    ESP_LOGI("TAG", "Acquired in main task");

}
