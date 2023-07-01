# FreeRTOS Binging For ESP-IDF

This is a FreeRTOS C++ binding. Supports some advanced C++ features.

The current supported FreeRTOS features: Task, Queue, Semaphores and Mutex.

# Contents

- [FreeRTOS Binging For ESP-IDF](#freertos-binging-for-esp-idf)
- [Installation](#installation)
- [Getting Started](#getting-started)
  - [1. Tasks](#1-tasks)
    - [(1) Creating](#1-creating)
      - [a) Without Arguments:](#a-without-arguments)
      - [b) With Arguments:](#b-with-arguments)
    - [(2) Deleting.](#2-deleting)
      - [Delete itself](#delete-itself)
      - [Delete by `task<...>` object](#delete-by-task-object)
    - [(3) Get Native Task Handle (TaskHandle_t)](#3-get-native-task-handle-taskhandle_t)
    - [(4) Reference count.](#4-reference-count)
  - [2. Queue](#2-queue)
  - [3. Semaphores and Mutex](#3-semaphores-and-mutex)


# Installation

```yaml
# idf_component.yml
dependencies:
  freertos-cpp:
    git: https://github.com/Augtons/ESP-FreeRTOS-Cpp
```

# Getting Started

## 1. Tasks

Please refer to examples:
 - `get-started`, [Click Here](examples/get-started/main/get-started.cpp)

 - `reference_count`, [Click Here](examples/reference_count/main/reference_count.cpp)

### (1) Creating

#### a) Without Arguments:

```cpp
task<> your_task = task_builder<>("task name")
    .stack(2048)
    .priority(0)
    .bind([] {
        while (true) {
            ESP_LOGI("TAG", "Task is running.");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
```

```cpp
task<> your_task = task_factory<>::create("task name", 2048, 0, [] {
    while (true) {
        ESP_LOGI("TAG", "Task is running.");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
});
```

Also Supports function.

```cpp

void task_function() {
    while (true) {
        ESP_LOGI("TAG", "Task is running.");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

task<> your_task = task_factory<>::create("task name", 2048, 0, task_function);
```

#### b) With Arguments:

Next, using C++ reference types as an example, please note that during the task execution, the reference must remain valid (for example, int a = 0 in the following example must not go out of scope)

```cpp
int a = 123;
task<int&> your_task = task_builder<int&>("task name")
    .stack(2048)
    .priority(0)
    .bind(a, [](int& args) { // variable `a` will be passed as a int&
        while (true) {
            ESP_LOGI("TAG", "Task is running, a = %d.", arg);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
```

You can also use lambda to capture variables in scope.
```cpp
int a = 123;
task<> your_task = task_builder<>("task name")
    .stack(2048)
    .priority(0)
    .bind([&a]() { // variable `a` captured as reference.
        while (true) {
            ESP_LOGI("TAG", "Task is running, a = %d.", a);
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    });
```

`task_factory<Type>` also ok! 

```cpp
int a = 123;
task<int&> your_task = task_factory<int&>::create("task name", 2048, 0, a, [](int& arg) {
    while (true) {
        ESP_LOGI("TAG", "Task is running, a = %d.", arg);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
});
```

### (2) Deleting.

#### Delete itself

It will be deleted automatically when its task function returns.

The Safe Way:

```cpp
task<> your_task = task_factory<>::create("task name", 2048, 0, [] {
    ESP_LOGI("TAG", "Start.");
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI("TAG", "Will be deleted.");
});
```

> Note: **Don't** use `vTaskDelete(NULL)` to delete it! Memory leaks will occur.

#### Delete by `task<...>` object

The Safe Way:

```cpp
your_task.delete_task();
```

> Note: **Don't** use `vTaskDelete((TaskHandle_t)your_task)` to delete it. Memory leaks will occur.


### (3) Get Native Task Handle (TaskHandle_t)

```cpp
// The first way.
auto handle = (TaskHandle_t)your_task;
// Equals to the first way.
auto handle = your_task.native_handle();

// Get native handle but `abort()` when your_task is null.
auto handle = your_task.native_handle_not_null();

```

Examples

```cpp
// 1.
task<> t1;
auto handle = (TaskHandle_t)t1; // handle is nullptr.
auto handle = t1.native_handle(); //handle is nullptr.

// 2.
task<> t2;
task<> t3 = task_builder<>(...)...;   // Create Task

auto handle = t2.native_handle_not_null();  // Error, abort().
auto handle = t3.native_handle_not_null();  // Handle of t3.
auto handle = (TaskHandle_t)t2;             // nullptr
```


### (4) Reference count.

When the reference count of a `task<>` object is 0, the FreeRTOS Task will be deleted.

It has some features similar to `std::shared_ptr`.

```cpp
task<> t1;
t1.use_count();   // use_count of null object it always 1.

task<> t2 = task_builder<>("task").stack(2048).priority(0).bind([]() {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
});
t2.use_count();   // 1
t1 = t2;
t2.use_count();   // 2
t1.use_count();   // 2

task<> t3 = task_builder<>("task2").stack(2048).priority(0).bind([]() {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
});

t1 = t3;
t3.use_count();   // 2
t2.use_count();   // 1

task<> t4 = task_builder<>("task2").stack(2048).priority(0).bind([]() {
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
});
t1 = std::move(t4);
t1.use_count();   // 1

```

## 2. Queue

Please refer to examples `queue`, [Click Here](examples/queue/main/queue.cpp)

## 3. Semaphores and Mutex

Please refer to examples `semaphore`, [Click Here](examples/semaphore/main/semaphore.cpp)
