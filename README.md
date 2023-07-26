# Универсальный интерфейс для Cyphal

## API

*TODO*.
Сейчас пример с комментариями можно найти тут:
[пример для ардуино с подробными комментами](examples/hbeat-arduino/main.ino).

## Платформы

Инструкции по установке и сборке под конкретные платформы:

### Arduino

Клонируйте данный репозитории в папку с библиотеками (`libraries`). 
Туда же скачайте еще вот эти библиотеки - [STM32duino-Libcanard](https://github.com/voltbro/STM32duino-Libcanard2)
и [VBCoreG4](https://github.com/VBCores/VBCoreG4_arduino_system).

**Пример**: [hbeat-arduino](examples/hbeat-arduino/main.ino).

### STM32 (CubeIDE)

Клонируйте данный репозиторий в любую папку. Я обычно добавляю в папку `Drivers` и добавляю путь `Drivers/libcyphal` в `include directories` в настройках билда.
Папку `libcyphal/src` надо исключить из билда (правая кнопка - exclude). Все, можно пользоваться.

**Пример** (полный проект): [hbeat-stm](examples/hbeat-stm).

Самые важные части примера:
- **Файл** где используется эта библиотека: [main_impl.cpp](examples/hbeat-stm/Core/Src/main_impl.cpp)
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
get_filename_component(CYPHAL_DIR "../libcyphal" ABSOLUTE)
message("${CMAKE_CURRENT_LIST_DIR} | Using <${CYPHAL_DIR}> as directory of libcyphal")

get_filename_component(COMMON_LIBS_DIR "../libcyphal/libs" ABSOLUTE)
message("${CMAKE_CURRENT_LIST_DIR} | Using <${COMMON_LIBS_DIR}> as directory of common Cyphal libs - uavcan, reg, etc.")

include_directories(${CYPHAL_DIR} ${COMMON_LIBS_DIR})

add_subdirectory(${CYPHAL_DIR} ${PROJECT_BINARY_DIR}/build/libcyphal)

target_link_libraries(YOUR_PROJECT_NAME libcyphal)
```
Так как uavcan и все прочее header-only, их билдить и линковать не надо, достаточно `include_directories`.

**Пример**: [hbeat-linux](examples/hbeat-linux).

**Более сложный пример использования в реальном проекте**: [vbcores/ros/movement_control](https://github.com/voltbro/vbcores/tree/master/ros/src/movement_control). 
(*TODO*: на самом деле там сейчас еще моя старая реализация, скоро переведу на новую).
