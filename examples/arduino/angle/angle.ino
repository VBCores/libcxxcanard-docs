#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/si/sample/angle/Scalar_1_0.h>

TYPE_ALIAS(AngleScalar, uavcan_si_sample_angle_Scalar_1_0)

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

#define ANGLE_PORT_ID 1111
void send_angle(float radian) {
    static uint8_t angle_buffer[AngleScalar::buffer_size];
    static CanardTransferID angle_transfer_id = 0;
    AngleScalar::Type angle_msg = {
        .timestamp = {.microsecond=micros_64()},
        .radian = radian
    };
    interface->send_msg<AngleScalar>(&angle_msg, angle_buffer, ANGLE_PORT_ID, &angle_transfer_id);
}

uint64_t last_send = 0;
#define MICROS_SEC 1000000
void loop() {
    interface->loop();

    auto now = micros_64();
    if ( (now - last_send) > MICROS_SEC ) {
        last_send = now;
        float some_angle = 1;
        send_angle(some_angle);
    }
}
