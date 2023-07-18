#include "cyphal/allocators/o1/o1_allocator.h"
#include "cyphal/cyphal.h"
#include "cyphal/providers/LinuxCAN.h"
#include "cyphal/subscriptions/subscription.h"

#include <iostream>

#include "uavcan/node/Heartbeat_1_0.h"

void Error_Handler() {
    std::cout << "err" << std::endl;
}

SUBSCRIPTION_CLASS_FIXED_MESSAGE(HBeatReader, uavcan_node_Heartbeat_1_0)
void HBeatReader::handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) {
    std::cout << hbeat.uptime << " " << +transfer->metadata.remote_node_id << std::endl;
}

int main() {
    auto interface = CyphalInterface(99);
    interface.setup<LinuxCAN, O1Allocator>("can0");
    auto reader = HBeatReader(&interface);
    while (1) {
        interface.loop();
    }
}