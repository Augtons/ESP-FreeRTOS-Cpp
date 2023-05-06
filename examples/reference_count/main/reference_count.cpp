#include <iostream>
#include "esp_log.h"
#include "freertoscpp/freertos.hpp"
#include "freertoscpp/freertos_task_factory.hpp"

using namespace std;
using augtons::freertos::task;
using augtons::freertos::task_factory;
using augtons::freertos::task_builder;

extern "C" void app_main()
{
    task<> t1;
    cout << t1.use_count() << endl;   // use_count of null object it always 1.

    task<> t2 = task_builder<>("task").stack(2048).priority(0).bind([]() {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
    cout << t2.use_count() << endl;   // 1
    t1 = t2;
    cout << t2.use_count() << endl;   // 2
    cout << t1.use_count() << endl;   // 2

    task<> t3 = task_builder<>("task2").stack(2048).priority(0).bind([]() {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
    t1 = t3;
    cout << t3.use_count() << endl;   // 2
    cout << t2.use_count() << endl;   // 1

    task<> t4 = task_builder<>("task2").stack(2048).priority(0).bind([]() {
        while (true) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
    t1 = std::move(t4);
    cout << t1.use_count() << endl;   // 1

}
