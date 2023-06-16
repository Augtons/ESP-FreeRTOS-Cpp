#include <iostream>
#include "esp_log.h"
#include "freertoscpp/freertos.hpp"
#include "freertoscpp/freertos_task_factory.hpp"
#include "freertoscpp/queue.hpp"

using namespace std;
using augtons::freertos::task;
using augtons::freertos::queue;
using augtons::freertos::task_factory;
using augtons::freertos::task_builder;

struct Test {
    int a = 4;
    std::string str = "asdaf";

    Test() = default;
    Test(Test&) = delete;
    Test(Test&&) = default;
};

extern "C" void app_main()
{
    queue<Test> q(32);

    auto task1 = task_builder<void>("task1").stack(2048).priority(0).bind([=]() {
        while (1) {
            q.send(Test());
            ESP_LOGI("TAG", "Sent! ref_count = %ld", q.use_count());  // It's 2, because of lambda capture.
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });

    while (1) {
        auto v = q.receive_forever();
        ESP_LOGI("TAG", "Received[%ld]: %s", q.use_count(), v.str.c_str());
        ESP_LOGI("TAG", "Received[%ld]: %d", q.use_count(), v.a);
        ESP_LOGI("TAG", "FreeHeap: %.3f M", (float)esp_get_free_heap_size() / 1024 / 1024);
    }

}
