#include "main.h"

#include <memory>

#include "cyphal/cyphal.h"
#include "cyphal/providers/G4CAN.h"
#include "cyphal/allocators/sys/sys_allocator.h"
#include "cyphal/subscriptions/subscription.h"

#include "uavcan/node/Heartbeat_1_0.h"

std::byte buffer[sizeof(CyphalInterface) + sizeof(G4CAN) + sizeof(SystemAllocator)];
std::shared_ptr<CyphalInterface> interface;

uint32_t uptime = 0;
PREPARE_MESSAGE(uavcan_node_Heartbeat_1_0, hbeat)

void error_handler() {
    Error_Handler();
}

uint64_t micros_64() {
	// Тут не нужен точный таймер, поэтому так
	return HAL_GetTick() * 1000;
}

extern "C" {
void heartbeat() {
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {
        .uptime = uptime,
        .health = {uavcan_node_Health_1_0_NOMINAL},
        .mode = {uavcan_node_Mode_1_0_OPERATIONAL}
    };
    interface->SEND_MSG(
        uavcan_node_Heartbeat_1_0,
        &heartbeat_msg,
        hbeat_buf,
        uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
        &hbeat_transfer_id
    );
    uptime += 1;
}

void setup_cyphal(FDCAN_HandleTypeDef* handler) {
	interface = std::shared_ptr<CyphalInterface>(
		CyphalInterface::create<G4CAN, SystemAllocator>(buffer, 98, handler, 400)  // 400 msgs combined pool
	);
}

void cyphal_loop() {
    interface->loop();
}

}
