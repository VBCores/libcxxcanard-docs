#include "cyphal/allocators/o1/o1_allocator.h"
#include "cyphal/cyphal.h"
#include "cyphal/providers/LinuxCAN.h"
#include "cyphal/subscriptions/subscription.h"

#include <iostream>
#include <chrono>

#include "uavcan/node/Heartbeat_1_0.h"

void Error_Handler() {std::cout << "error" << std::endl; while(1){}}

SUBSCRIPTION_CLASS_FIXED_MESSAGE(HBeatReader, uavcan_node_Heartbeat_1_0)
void HBeatReader::handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) {
    std::cout << +transfer->metadata.remote_node_id << ": " << hbeat.uptime <<  std::endl;
}

CyphalInterface* interface;

uint32_t uptime = 0;
PREPARE_MESSAGE(uavcan_node_Heartbeat_1_0, hbeat)
void heartbeat() {
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {.uptime = uptime, .health = {uavcan_node_Health_1_0_NOMINAL}, .mode = {uavcan_node_Mode_1_0_OPERATIONAL}};
    interface->SEND_MSG(uavcan_node_Heartbeat_1_0, &heartbeat_msg, hbeat_buf, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, &hbeat_transfer_id);
    uptime += 1;
}

uint64_t current_time_millis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main() {
    interface = new CyphalInterface(100);
    interface->setup<LinuxCAN, O1Allocator>("can0");
    auto reader = HBeatReader(interface);

    auto last_hbeat = current_time_millis();
    while (1) {
        interface->loop();
        auto now = current_time_millis();
        if ( (now - last_hbeat) >= 1000) {
            last_hbeat = now;
            heartbeat();
        }
    }
}