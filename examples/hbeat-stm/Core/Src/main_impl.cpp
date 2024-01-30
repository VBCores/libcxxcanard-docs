#include "main.h"

#include <memory>

#include "cyphal/cyphal.h"
#include "cyphal/providers/G4CAN.h"
#include "cyphal/allocators/o1/o1_allocator.h"
#include "cyphal/subscriptions/subscription.h"

#include "uavcan/node/Heartbeat_1_0.h"
TYPE_ALIAS(HBeat, uavcan_node_Heartbeat_1_0)

std::byte buffer[sizeof(CyphalInterface) + sizeof(G4CAN) + sizeof(O1Allocator)];
std::shared_ptr<CyphalInterface> interface;

void error_handler() { Error_Handler(); }
// Тут не нужен точный таймер, поэтому так
uint64_t micros_64() { return HAL_GetTick() * 1000; }
UtilityConfig utilities(micros_64, error_handler);

extern "C" {
void heartbeat() {
	static uint8_t hbeat_buffer[HBeat::buffer_size];
	static CanardTransferID hbeat_transfer_id = 0;
	static uint32_t uptime = 0;
    uavcan_node_Heartbeat_1_0 heartbeat_msg = {
        .uptime = uptime,
        .health = {uavcan_node_Health_1_0_NOMINAL},
        .mode = {uavcan_node_Mode_1_0_OPERATIONAL}
    };
    interface->send_msg<HBeat>(
		&heartbeat_msg,
		hbeat_buffer,
		uavcan_node_Heartbeat_1_0_FIXED_PORT_ID_,
		&hbeat_transfer_id,
		3000000
	);
    uptime += 1;
}

void setup_cyphal(FDCAN_HandleTypeDef* handler) {
	interface = std::shared_ptr<CyphalInterface>(
		// memory location, node_id, fdcan handler, messages memory pool, utils ref
		CyphalInterface::create<G4CAN, O1Allocator>(buffer, 98, handler, 400, utilities)
	);
}

void cyphal_loop() {
    interface->loop();
}

}

