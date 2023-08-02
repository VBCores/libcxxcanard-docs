#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/primitive/String_1_0.h>

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

const CanardPortID SD_PORT = 176;                 // 'S'elf 'D'iagnostic - 8368, 8368 % 8192 = 176
PREPARE_MESSAGE(uavcan_primitive_String_1_0, sd)  // создаст sd_buf, sd_transfer_id
void send_diagnostic(char* string) {              // строка должна быть нуль-терминирована
    uavcan_primitive_String_1_0 sd = {};
    sprintf((char*)sd.value.elements, "%s", string);
    sd.value.count = strlen((char*)sd.value.elements);

    interface->SEND_MSG(uavcan_primitive_String_1_0, &sd, sd_buf, SD_PORT, &sd_transfer_id);
}

uint64_t last_send = 0;
#define MICROS_SEC 1000000
void loop() {
    interface->loop();

    auto now = micros_64();
    if ( (now - last_send) > MICROS_SEC ) {
        last_send = now;
        send_diagnostic("diagnostic string\0");
    }
}
