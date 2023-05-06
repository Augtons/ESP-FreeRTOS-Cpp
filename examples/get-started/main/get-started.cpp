#include <iostream>
#include "esp_log.h"
#include "freertoscpp/freertos.hpp"
#include "freertoscpp/freertos_task_factory.hpp"

using namespace std;
using augtons::freertos::task;
using augtons::freertos::task_factory;
using augtons::freertos::task_builder;

const char *TAG = "MAIN";

static task<> my_task1;
static task<int&> my_task2;

void task_1_func();
void task_2_func(int& arg);

extern "C" void app_main()
{
    /*** 1: Use Function ***/
    my_task1 = task_builder<>("my_task1").stack(2048).priority(0).bind(task_1_func);
    // Same as:
    // my_task1 = task_factory<>::create("my_task1", 2048, 0, task_1_func);

    int abcdef = 111;
    my_task2 = task_builder<int&>("my_task").stack(2048).priority(0).bind(abcdef, task_2_func);
    // Same as:
    //my_task2 = task_factory<int&>::create("my_task", 2048, 0, abcdef, task_2_func);

    vTaskDelay(pdMS_TO_TICKS(5000));
    my_task1 = nullptr;

    /***************************************************************************/

    /*** 2: Use Lambda ***/
    my_task2 = task_builder<int&>("my_task1").stack(2048).priority(0).bind(abcdef, [](int& arg) {
        while (true) {
            ESP_LOGI(TAG, "my_task2 is running, adcdef = %d", arg);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
    abcdef += 123;   // Change abcdef to 234!!

    /*** 3: A Simple Usage ***/
    my_task1 = task_builder<>("my_task1_test2").stack(2048).priority(1).bind([]() {
        auto url = "www.baidu.com";

        auto task = task_builder<>("temp").stack(2048).priority(0).bind([url]() {
            // Create a low-priority task, execute a time-consuming task.
            // And signal an outer task through a task notification.
            ESP_LOGI(TAG, "Start! url = %s", url);
            vTaskDelay(pdMS_TO_TICKS(500));
            xTaskNotifyGive((TaskHandle_t)my_task1);
        });

        // Waiting for time-consuming task finish.
        if(ulTaskNotifyTake(true, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Finish");
        }
    });

}


void task_1_func() {
    while (true) {
        ESP_LOGI(TAG, "my_task1 is running");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void task_2_func(int& arg) {
    while (true) {
        ESP_LOGI(TAG, "my_task2 is running, arg = %d", arg);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}