#include <VBCoreG4_arduino_system.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/primitive/String_1_0.h>

TYPE_ALIAS(CyphalString, uavcan_primitive_String_1_0)


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

const CanardPortID SD_PORT = 176;    // 'S'elf 'D'iagnostic - 8368, 8368 % 8192 = 176
void send_diagnostic(char* string) { // строка должна быть нуль-терминирована
    static uint8_t sd_buf[CyphalString::buffer_size];
    static CanardTransferID sd_transfer_id = 0;
    uavcan_primitive_String_1_0 sd = {};
    sprintf((char*)sd.value.elements, "%s", string);
    sd.value.count = strlen((char*)sd.value.elements);

    interface->send_msg<CyphalString>(&sd, sd_buf, SD_PORT, &sd_transfer_id);
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
