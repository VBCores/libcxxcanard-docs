#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/primitive/array/Real32_1_0.h>

#include <array>

TYPE_ALIAS(ArrayFloat, uavcan_primitive_array_Real32_1_0)

CanFD canfd;
FDCAN_HandleTypeDef* hfdcan1;

std::shared_ptr<CyphalInterface> interface;

void error_handler() {Serial.println("error"); while (1) {};}
uint64_t micros_64() {return micros();}
UtilityConfig utilities(micros_64, error_handler);

void setup() {
    canfd.can_init();
    hfdcan1 = canfd.get_hfdcan();

    interface = CyphalInterface::create_heap<G4CAN, O1Allocator>(97, hfdcan1, 200, utilities);
}

#define ARRAY_PORT_ID 2222
template<typename Container>
void send_numbers(Container& container) {
    static uint8_t array_buf[ArrayFloat::buffer_size];
    static CanardTransferID array_transfer_id = 0;
    ArrayFloat::Type array_msg = {};
    array_msg.value.count = container.size();

    float* start = array_msg.value.elements;
    int pos = 0;
    for (float elem : container) {
        *(start + pos) = elem;
        pos++;
    }
    interface->send_msg<ArrayFloat>(&array_msg, array_buf, ARRAY_PORT_ID, &array_transfer_id);
}

uint64_t last_send = 0;
#define MICROS_SEC 1000000
void loop() {
    interface->loop();

    auto now = micros_64();
    if ( (now - last_send) > MICROS_SEC ) {
        last_send = now;
        std::array<float, 3> three_numbers = {1.2, 3.4, 5.6};  // можно послать любой (почти) другой контейнер
        send_numbers(three_numbers);
    }
}
