#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/primitive/array/Real32_1_0.h>

#include <array>

CanFD canfd;
FDCAN_HandleTypeDef* hfdcan1;

CyphalInterface* interface;

void error_handler() {Serial.println("error"); while (1) {};}
uint64_t micros_64() {return micros();}

void setup() {
    canfd.can_init();
    hfdcan1 = canfd.get_hfdcan();

    interface = new CyphalInterface(99);
    interface->setup<G4CAN, SystemAllocator>(hfdcan1);
}

PREPARE_MESSAGE(uavcan_primitive_array_Real32_1_0, array)  // создаст array_buf, array_transfer_id
#define ARRAY_PORT_ID 2222
template<typename Container>
void send_numbers(Container& container) {
    uavcan_primitive_array_Real32_1_0 array_msg = {};
    array_msg.value.count = container.size();

    float* start = array_msg.value.elements;
    int pos = 0;
    for (float elem : container) {
        *(start + pos) = elem;
        pos++;
    }
    interface->SEND_MSG(uavcan_primitive_array_Real32_1_0, &array_msg, array_buf, ARRAY_PORT_ID, &array_transfer_id);
}

uint64_t last_send = 0;
#define MICROS_SEC 1000000000
void loop() {
    interface->loop();

    auto now = micros_64();
    if ( (now - last_send) > MICROS_SEC ) {
        last_send = now;
        std::array<float, 3> three_numbers = {1.2, 3.4, 5.6};  // можно послать любой (почти) другой контейнер
        send_numbers(three_numbers);
    }
}
