#include <iostream>
#include <chrono>
#include <cstdlib>
#include <memory>

#include "cyphal/allocators/o1/o1_allocator.h"
#include "cyphal/cyphal.h"
#include "cyphal/providers/LinuxCAN.h"
#include "cyphal/subscriptions/subscription.h"

#include "uavcan/node/Heartbeat_1_0.h"

std::shared_ptr<CyphalInterface> interface;

void error_handler() {std::cout << "error" << std::endl; std::exit(EXIT_FAILURE);}

uint64_t timeMillis() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

SUBSCRIPTION_CLASS_FIXED_MESSAGE(HBeatReader, uavcan_node_Heartbeat_1_0)
void HBeatReader::handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) {
    std::cout << +transfer->metadata.remote_node_id << ": " << hbeat.uptime <<  std::endl;
}

uint32_t uptime = 0;
PREPARE_MESSAGE(uavcan_node_Heartbeat_1_0, hbeat)
void heartbeat() {
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {.uptime = uptime, .health = {uavcan_node_Health_1_0_NOMINAL}, .mode = {uavcan_node_Mode_1_0_OPERATIONAL}};
    interface->SEND_MSG(uavcan_node_Heartbeat_1_0, &heartbeat_msg, hbeat_buf, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, &hbeat_transfer_id);
    uptime += 1;
}

int main() {
    interface = std::make_shared<CyphalInterface>(100);
    interface->setup<LinuxCAN, O1Allocator>("can0", 8*1024*10);  // 10 kb memory pool

    auto reader = HBeatReader(interface);

    auto last_hbeat = timeMillis();
    while (1) {
        interface->loop();
        auto now = timeMillis();
        if ( (now - last_hbeat) >= 1000) {
            last_hbeat = now;
            heartbeat();
        }
    }
}