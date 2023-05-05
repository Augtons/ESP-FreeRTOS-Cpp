#include <iostream>
#include "vector"
#include "esp_log.h"
#include "freertoscpp/freertos.hpp"
#include "freertoscpp/freertos_task_factory.hpp"

using namespace std;
using augtons::freertos::task;
using augtons::freertos::task_factory;
using augtons::freertos::task_builder;

const char *TAG = "MAIN";

void print_tasks() {
    char buf[256] = {0};
    vTaskList(buf);
    std::cout << buf << "\n" << std::endl;
}

extern "C" void app_main()
{
    ESP_LOGI(TAG, "程序启动!");

    auto tasks = vector<task<>>();

    for(int i = 0; i < 10; i++) {
        string task_name = string("Task") + to_string(i);

        auto t = task_builder<>(task_name.c_str()).stack(2048)
            .priority(0)
            .bind([=] {
                while (1) {
                    ESP_LOGI(TAG, "任务 %d 正在运行", i);
                    vTaskDelay(pdMS_TO_TICKS(1000));
                }
            });

        tasks.push_back(std::move(t));

        ESP_LOGI(TAG, "添加任务：%s!", task_name.c_str());
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI(TAG, "容器即将被析构，任务即将自动被删除");

}
