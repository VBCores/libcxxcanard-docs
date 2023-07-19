#include <VBCoreG4_arduino_system.h>
#include <stm32g4xx_hal_fdcan.h>
#include <Libcanard2.h>
#include <cyphal.h>
#include <uavcan/node/Heartbeat_1_0.h>

uint32_t uptime = 0;
HardwareTimer *timer = new HardwareTimer(TIM3);
CyphalInterface* interface;

void error_handler() {Serial.println("error"); while (1) {};}

SUBSCRIPTION_CLASS_FIXED_MESSAGE(HBeatReader, uavcan_node_Heartbeat_1_0)
void HBeatReader::handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) {
    Serial.print(+transfer->metadata.remote_node_id);
    Serial.print(": ");
    Serial.println(hbeat.uptime);
}
HBeatReader* reader;

void setup() {
    can_init();
    interface = new CyphalInterface(99);
    interface->setup<G4CAN, SystemAllocator>(&hfdcan1);
    reader = new HBeatReader(interface);

    timer->pause();
    timer->setPrescaleFactor(7999);
    timer->setOverflow(19999);
    timer->attachInterrupt(hbeat_func);
    timer->refresh();
    timer->resume();
}

void loop() {
    interface->loop();
}

PREPARE_MESSAGE(uavcan_node_Heartbeat_1_0, hbeat)
void send_heartbeat() {
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {.uptime = uptime, .health = {uavcan_node_Health_1_0_NOMINAL}, .mode = {uavcan_node_Mode_1_0_OPERATIONAL}};
    interface->SEND_MSG(uavcan_node_Heartbeat_1_0, &heartbeat_msg, hbeat_buf, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, &hbeat_transfer_id);
}

void hbeat_func() {
    digitalWrite(LED2, !digitalRead(LED2));
    send_heartbeat();
    uptime += 1;
}