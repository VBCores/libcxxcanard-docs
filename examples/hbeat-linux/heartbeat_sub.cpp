#include <iostream>
#include <chrono>
#include <cstdlib>
#include <memory>

#include "cyphal/allocators/o1/o1_allocator.h"
#include "cyphal/cyphal.h"
#include "cyphal/providers/LinuxCAN.h"
#include "cyphal/subscriptions/subscription.h"

#include "uavcan/node/Heartbeat_1_0.h"
TYPE_ALIAS(HBeat, uavcan_node_Heartbeat_1_0)


void error_handler() {std::cout << "error" << std::endl; std::exit(EXIT_FAILURE);}

uint64_t micros_64() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

UtilityConfig utilities(micros_64, error_handler);
std::byte buffer[sizeof(CyphalInterface) + sizeof(LinuxCAN) + sizeof(O1Allocator)];
std::shared_ptr<CyphalInterface> interface;

class HBeatReader: public AbstractSubscription<HBeat> {
public:
    HBeatReader(InterfacePtr interface): AbstractSubscription<HBeat>(interface, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_) {};
    void handler(const uavcan_node_Heartbeat_1_0& hbeat, CanardRxTransfer* transfer) override {
        std::cout << +transfer->metadata.remote_node_id << ": " << hbeat.uptime <<  std::endl;
    }
};

uint8_t hbeat_buffer[HBeat::buffer_size];
CanardTransferID hbeat_transfer_id = 0;
uint32_t uptime = 0;
void heartbeat() {
    HBeat::Type heartbeat_msg = {.uptime = uptime, .health = {uavcan_node_Health_1_0_NOMINAL}, .mode = {uavcan_node_Mode_1_0_OPERATIONAL}};
    interface->send_cyphal_default_msg<HBeat>(&heartbeat_msg, hbeat_buffer, uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_, &hbeat_transfer_id);
    uptime += 1;
}

int main() {
    interface = std::shared_ptr<CyphalInterface>(
        CyphalInterface::create<LinuxCAN, O1Allocator>(buffer, 100, "can0", 1000, utilities)  // 1000 msgs combined pool
    );

    auto reader = HBeatReader(interface);

    auto last_hbeat = micros_64();
    while (1) {
        interface->loop();
        auto now = micros_64();
        if ( (now - last_hbeat) >= 1000) {
            std::cout << "Sending heartbeat " << uptime << ". Queue size: " << interface->queue_size() << std::endl;
            last_hbeat = now;
            heartbeat();
        }
    }
}