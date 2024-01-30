#include <memory>
#include <VBCoreG4_arduino_system.h>   // системный хэдер
#include <stm32g4xx_hal_fdcan.h>       // системный хэдер
#include <Libcanard2.h>                // базовые библиотеки (canard, uavcan, etc.)
#include <cyphal.h>                    // сам libcxxcanard
#include <uavcan/node/Heartbeat_1_0.h> // тип сообщения, которе будем использовать

// С++ обертка вокруг кучи libcanard-типов
TYPE_ALIAS(HBeat, uavcan_node_Heartbeat_1_0)

// Во избежание аллокаций на куче
std::byte buffer[sizeof(CyphalInterface) + sizeof(G4CAN) + sizeof(O1Allocator)];

// Настройка fdcan из VBCoreG4
CanFD canfd;
FDCAN_HandleTypeDef* hfdcan1;

// Таймер, по прерыванию которого будем посылать сообщения
uint32_t uptime = 0;
HardwareTimer *timer = new HardwareTimer(TIM3);

// Класс с API для доступа к cyphal
std::shared_ptr<CyphalInterface> interface;

// Объявим функцию, которую libcyphal будем вызывать для обработки ошибок
void error_handler() {Serial.println("error"); while (1) {};}
uint64_t micros_64() {return micros();}
UtilityConfig utilities(micros_64, error_handler);

class HBeatReader: public AbstractSubscription<HBeat> {
public:
    HBeatReader(InterfacePtr interface): AbstractSubscription<HBeat>(interface,
        // Тут параметры - port_id, transfer kind или только port_id
        uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_
    ) {};
    void handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) override {
        Serial.print(+transfer->metadata.remote_node_id);
        Serial.print(": ");
        Serial.println(hbeat.uptime);
    }
};

HBeatReader* reader;

void setup() {
    // запускаем can
    canfd.can_init();
    hfdcan1 = canfd.get_hfdcan();

    interface = std::shared_ptr<CyphalInterface>(
        // memory location, node_id, fdcan handler, messages memory pool, utils ref
        CyphalInterface::create<LinuxCAN, O1Allocator>(buffer, 97, hfdcan1, 200, utilities)
    );
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
 * Макрос, прячущий шаблонный код для объявления буфера и transfer_id для ОТПРАВКИ сообщений
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
 * cyphal/cyphal.h - для обработки и отправки сообщений
 *
 * Иначе можно просто честно наследоваться от классов и писать все самому (пример выше в комментариях).
 *
 * Отдельный PREPARE_MESSAGE - чтоб объявить буферы и тд.
 * Он самый спорный тк создает глобальные переменные и экономит всего одну строку. Пример и с ним, и без него есть выше,
 * решайте сами пользоваться или нет.
 */
