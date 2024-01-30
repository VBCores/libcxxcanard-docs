# Универсальный интерфейс для Cyphal

## API

*TODO*.
Сейчас пример с комментариями можно найти тут:
[пример для ардуино с подробными комментами](examples/arduino/heartbeat/heartbeat.ino).

## Платформы

Инструкции по установке и сборке под конкретные платформы:

### Arduino

Клонируйте данный репозитории в папку с библиотеками (`libraries`). 
Туда же скачайте еще вот эти библиотеки - [STM32duino-Libcanard](https://github.com/voltbro/STM32duino-Libcanard2)
и [VBCoreG4](https://github.com/VBCores/VBCoreG4_arduino_system).

**Примеры**: [arduino](examples/arduino).

### STM32 (CubeIDE)

Клонируйте данный репозиторий в любую папку. Я обычно добавляю в папку `Drivers` и добавляю путь `Drivers/libcxxcanard` в `include directories` в настройках билда.
Папку `libcxxcanard/src` надо исключить из билда (*правая кнопка -> resourcce configurations -> exclude*). Все, можно пользоваться.

**Пример** (полный проект): [hbeat-stm](examples/hbeat-stm).

Самые важные части примера:
- **Файл** где используется эта библиотека: [main_impl.cpp](examples/hbeat-stm/Core/Src/main_impl.cpp). Тут инициализация, отправка сообщений и прочее
- Код в main, вызывающий контрольный цикл и посылающий heartbeat:
```c
uint32_t last_hbeat = HAL_GetTick();
while (1)
{
// cyphal_loop - объявлен main.h,
// реализован в main_impl.cpp (одна строка, вызов библиотеки)
    cyphal_loop();

    uint32_t now = HAL_GetTick();
    if ( (now - last_hbeat) >= 1000) {
        last_hbeat = now;
        heartbeat();  // аналогично
    }
}
```

### Linux (CMake)

Клонируйте данный репозиторий в любую папку, я обычно кладу в соседнюю с проектом. Укажите в `CMakeLists.txt` пути до libcyphal и общих библиотек (libcanard, o1heap, и т.д.).

Рабочий кусок cmake для добавления нужных библиотек (предполагается что libcyphal находится в соседней с проектом папке, 
но пути можно поменять, или брать из env как [тут](examples/hbeat-linux/CMakeLists.txt)). Шаблон:
```cmake
if(DEFINED ENV{<CYPHAL_DIR>})
    set(CYPHAL_DIR $ENV{CYPHAL_DIR})
else()
    get_filename_component(CYPHAL_DIR
                           "libs/libcxxcanard"
                           ABSOLUTE)
endif()
message("${CMAKE_CURRENT_LIST_DIR} | Using <${CYPHAL_DIR}> as directory of libcxxcanard")

add_subdirectory(${CYPHAL_DIR} ${PROJECT_BINARY_DIR}/build/libcxxcanard)

include_directories(... ${CYPHAL_DIR} ${COMMON_LIBS_DIR})
target_link_libraries(YOUR_PROJECT_NAME libcxxcanard)
```
Так как uavcan и все прочее header-only, их билдить и линковать не надо, достаточно `${COMMON_LIBS_DIR}` в `include_directories` (эта переменная создается при `add_subdirectory(${CYPHAL_DIR} ...)`).

**Пример**: [hbeat-linux](examples/hbeat-linux).

**Более сложный пример использования в реальном проекте**: TODO (выбрать).
