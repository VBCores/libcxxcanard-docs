#include <VBCoreG4_arduino_system.h>   // системный хэдер
#include <stm32g4xx_hal_fdcan.h>       // системный хэдер
#include <Libcanard2.h>                // базовые библиотеки (canard, uavcan, etc.)
#include <cyphal.h>                    // сам libcyphal
#include <uavcan/node/Heartbeat_1_0.h> // тип сообщения, которе будем использовать

// Настройка fdcan из VBCoreG4
CanFD canfd;
FDCAN_HandleTypeDef* hfdcan1;

// Таймер, по прерыванию которого будем посылать сообщения
uint32_t uptime = 0;
HardwareTimer *timer = new HardwareTimer(TIM3);

// Класс с API для доступа к cyphal
CyphalInterface* interface;

// Объявим функцию, которую libcyphal будем вызывать для обработки ошибок
void error_handler() {Serial.println("error"); while (1) {};}
uint64_t micros_64() {return micros();}

/*
 * В этой библиотеке много макросов для упрощения написания шаблонного кода - например, SUBSCRIPTION_CLASS_FIXED_MESSAGE
 * Куча других полезных макросов для объявления обработчиков есть в cyphal/subscriptions/subscription.h
 * Без макросов это можно сделать так:

class HBeatReader : public AbstractSubscription<uavcan_node_Heartbeat_1_0> {
private:
    inline void deserialize(uavcan_node_Heartbeat_1_0* object, CanardRxTransfer* transfer) {
        interface->DESERIALIZE_TRANSFER(uavcan_node_Heartbeat_1_0, object, transfer);
    }
public:
    HBeatReader(CyphalInterface* interface)
        : AbstractSubscription(
            interface,
            CanardTransferKindMessage,
            uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
            uavcan_node_Heartbeat_1_0_EXTENT_BYTES_
        ){};
    void handler(
        const uavcan_node_Heartbeat_1_0& hbeat,
        CanardRxTransfer* transfer
    ) override;
};

 * Как можно видеть, тут все еще остался макрос DESERIALIZE_TRANSFER, но его я уже рекомендую точно не избегать, тк это куча шаблонного кода
 * Список шаблонных функций и макросов к ним смотри в cyphal/cyphal.tpp
 * Позже я их задокументирую.
 */

// Макрос, объявляющий класс HBeatReader, обрабатывающий ПОЛУЧЕНИЕ uavcan_node_Heartbeat_1_0
SUBSCRIPTION_CLASS_FIXED_MESSAGE(HBeatReader, uavcan_node_Heartbeat_1_0)
// Если все же пользоваться макросами (что я рекомендую), то остается только реализовать функцию-обработчик получения сообщений:
// Сигнатура у них всегда (const ТИП_СООБЩЕНИЯ&, CanardRxTransfer*). Обратите внимание, что первое это константная ссылка, а второе указатель
void HBeatReader::handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) {
    Serial.print(+transfer->metadata.remote_node_id);
    Serial.print(": ");
    Serial.println(hbeat.uptime);
}
HBeatReader* reader;

void setup() {
    // запускаем can
    canfd.can_init();
    hfdcan1 = canfd.get_hfdcan();

    // "Запускаем" cyphal, id нашего узла будет 99
    interface = new CyphalInterface(99);
    // Инициализация - мы находимся на G4, алокатор памяти - системный
    // NOTE: в следующей версии, setup может пропасть и достаточно будет просто сделать
    // new CyphalInterface<G4CAN, SystemAllocator>(99, hfdcan1);
    interface->setup<G4CAN, SystemAllocator>(hfdcan1);
    // Создаем обработчик сообщений, который объявили выше
    reader = new HBeatReader(interface);

    // Настраиваем таймер на прерывания раз в секунду
    timer->pause();
    timer->setPrescaleFactor(7999);
    timer->setOverflow(19999);
    timer->attachInterrupt(hbeat_func);
    timer->refresh();
    timer->resume();
}

void loop() {
    // Вся обработка и fdcan, и cyphal, есть в CyphalInterface::loop
    interface->loop();
}

/*
 * Макрос, прячущий шаблонный код для объявления всякого для ОТПРАВКИ сообщений
 * Раскрывается в:

uint8_t hbeat_buf[uavcan_node_Heartbeat_1_0_EXTENT_BYTES_];
CanardTransferID hbeat_transfer_id = 0;

 * Объявлен в cyphal/cyphal.h
 */
PREPARE_MESSAGE(uavcan_node_Heartbeat_1_0, hbeat)
void send_heartbeat() {
    // Заполняем сообщение
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {.uptime = uptime, .health = {uavcan_node_Health_1_0_NOMINAL}, .mode = {uavcan_node_Mode_1_0_OPERATIONAL}};
    // Отправляем сообщение
    interface->SEND_MSG(uavcan_node_Heartbeat_1_0, &heartbeat_msg, hbeat_buf, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, &hbeat_transfer_id);
}

// Функция таймера
void hbeat_func() {
    digitalWrite(LED2, !digitalRead(LED2));
    send_heartbeat();
    uptime += 1;
}

/*
 * Итого:
 * 1) Создаем CyphalInterface с правильными параметрами для обработки fdcan/can и cyphal
 * 2) Объявляем классы-обработчики для сообщений, которые будем ПОЛУЧАТЬ
 * 3) Объявляем буферы и transfer_id для сообщений, которые будем ОТПРАВЛЯТЬ
 * 4) В цикле вызываем interface->loop
 * 5) ???
 * 6) Profit
 *
 * Код (особенно пункты 2 и 3) можно сильно сократить с помощью макросов.
 * cyphal/subscriptions/subscription.h - макросы для подписок на сообщения
 * cyphal/cyphal.tpp - для обработки и отправки сообщений
 *
 * Иначе можно просто честно наследоваться от классов и писать все самому (пример выше в комментариях).
 *
 * Отдельный PREPARE_MESSAGE - чтоб объявить буферы и тд.
 * Он самый спорный тк создает глобальные переменные и экономит всего одну строку. Пример и с ним, и без него есть выше,
 * решайте сами пользоваться или нет.
 */
