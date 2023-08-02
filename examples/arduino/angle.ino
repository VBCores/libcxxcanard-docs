#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/si/sample/angle/Scalar_1_0.h>

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

PREPARE_MESSAGE(uavcan_si_sample_angle_Scalar_1_0, angle)  // создаст angle_buf, angle_transfer_id
#define ANGLE_PORT_ID 1111
void send_angle(float radian) {
    uavcan_si_sample_angle_Scalar_1_0 angle_msg = {
        .radian = radian,
        .timestamp = {.microsecond=micros_64()}
    };
    interface->SEND_MSG(uavcan_si_sample_angle_Scalar_1_0, &angle_msg, angle_buf, ANGLE_PORT_ID, &angle_transfer_id);
}

uint64_t last_send = 0;
#define MICROS_SEC 1000000000
void loop() {
    interface->loop();

    auto now = micros_64();
    if ( (now - last_send) > MICROS_SEC ) {
        last_send = now;
        float some_angle = 1;
        send_angle(some_angle);
    }
}
