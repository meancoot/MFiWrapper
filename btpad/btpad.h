#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "btstack/btstack.h"
#include "btstack/utils.h"
#include "btstack/btstack.h"

void btpad_queue_reset();
void btpad_queue_run(uint32_t count);
void btpad_queue_process();
void btpad_queue_btstack_set_power_mode(uint8_t on);
void btpad_queue_hci_read_bd_addr();
void btpad_queue_hci_disconnect(uint16_t handle, uint8_t reason);
void btpad_queue_hci_inquiry(uint32_t lap, uint8_t length, uint8_t num_responses);
void btpad_queue_hci_remote_name_request(bd_addr_t bd_addr, uint8_t page_scan_repetition_mode, uint8_t reserved, uint16_t clock_offset);
void btpad_queue_hci_pin_code_request_reply(bd_addr_t bd_addr, bd_addr_t pin);
