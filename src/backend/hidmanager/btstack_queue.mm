/*  MFiWrapper
 *  Copyright (C) 2014 - Jason Fetters
 * 
 *  MFiWrapper is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  MFiWrapper is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with MFiWrapper.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "btstack.h"

struct btpad_queue_command
{
   const hci_cmd_t* command;

   union
   {
      struct
      {
         uint8_t on;
      }  btstack_set_power_mode;
   
      struct
      {
         bd_addr_t bd_addr;
         uint8_t role;
      }  hci_switch_role;
   
      struct
      {
         uint16_t handle;
         uint8_t reason;
      }  hci_disconnect;

      struct
      {
         bd_addr_t bd_addr;
         uint8_t page_scan_repetition_mode;
         uint8_t reserved;
         uint16_t clock_offset;
      }  hci_remote_name_request;

      struct // For wiimote only
      {
         bd_addr_t bd_addr;
         bd_addr_t pin;
      }  hci_pin_code_request_reply;
   };
};

struct btpad_queue_command commands[64];
static uint32_t insert_position;
static uint32_t read_position;
static uint32_t can_run;

#define INCPOS(POS) { POS##_position = (POS##_position + 1) % 64; }

void btpad_queue_reset()
{
   insert_position = 0;
   read_position = 0;
   can_run = 1;
}

void btpad_queue_run(uint32_t count)
{
   can_run = count;

   btpad_queue_process();
}

void btpad_queue_process()
{
   for (; can_run && (insert_position != read_position); can_run --)
   {
      struct btpad_queue_command* cmd = &commands[read_position];

           if (cmd->command == &btstack_set_power_mode)
         bt_send_cmd(cmd->command, cmd->btstack_set_power_mode.on);
      else if (cmd->command == &hci_switch_role_command)
         bt_send_cmd(cmd->command, cmd->hci_switch_role.bd_addr, cmd->hci_switch_role.role);
      else if (cmd->command == &hci_disconnect)
         bt_send_cmd(cmd->command, cmd->hci_disconnect.handle, cmd->hci_disconnect.reason);
      else if (cmd->command == &hci_remote_name_request)
         bt_send_cmd(cmd->command, cmd->hci_remote_name_request.bd_addr, cmd->hci_remote_name_request.page_scan_repetition_mode,
                         cmd->hci_remote_name_request.reserved, cmd->hci_remote_name_request.clock_offset);
      else if (cmd->command == &hci_pin_code_request_reply)
         bt_send_cmd(cmd->command, cmd->hci_pin_code_request_reply.bd_addr, 6, cmd->hci_pin_code_request_reply.pin);

      INCPOS(read);
   }
}

void btpad_queue_btstack_set_power_mode(uint8_t on)
{
   struct btpad_queue_command* cmd = &commands[insert_position];

   cmd->command = &btstack_set_power_mode;
   cmd->btstack_set_power_mode.on = on;

   INCPOS(insert);
   btpad_queue_process();
}

void btpad_queue_hci_switch_role(bd_addr_t bd_addr, uint8_t role)
{
   struct btpad_queue_command* cmd = &commands[insert_position];

   cmd->command = &hci_switch_role_command;
   memcpy(cmd->hci_switch_role.bd_addr, bd_addr, sizeof(bd_addr_t));
   cmd->hci_switch_role.role = role;

   INCPOS(insert);
   btpad_queue_process();
}

void btpad_queue_hci_disconnect(uint16_t handle, uint8_t reason)
{
   struct btpad_queue_command* cmd = &commands[insert_position];

   cmd->command = &hci_disconnect;
   cmd->hci_disconnect.handle = handle;
   cmd->hci_disconnect.reason = reason;

   INCPOS(insert);
   btpad_queue_process();
}

void btpad_queue_hci_remote_name_request(bd_addr_t bd_addr, uint8_t page_scan_repetition_mode, uint8_t reserved, uint16_t clock_offset)
{
   struct btpad_queue_command* cmd = &commands[insert_position];

   cmd->command = &hci_remote_name_request;
   memcpy(cmd->hci_remote_name_request.bd_addr, bd_addr, sizeof(bd_addr_t));
   cmd->hci_remote_name_request.page_scan_repetition_mode = page_scan_repetition_mode;
   cmd->hci_remote_name_request.reserved = reserved;
   cmd->hci_remote_name_request.clock_offset = clock_offset;

   INCPOS(insert);
   btpad_queue_process();
}

void btpad_queue_hci_pin_code_request_reply(bd_addr_t bd_addr, bd_addr_t pin)
{
   struct btpad_queue_command* cmd = &commands[insert_position];

   cmd->command = &hci_pin_code_request_reply;
   memcpy(cmd->hci_pin_code_request_reply.bd_addr, bd_addr, sizeof(bd_addr_t));
   memcpy(cmd->hci_pin_code_request_reply.pin, pin, sizeof(bd_addr_t));

   INCPOS(insert);
   btpad_queue_process();
}

//

static UIBackgroundTaskIdentifier btstackTaskID = UIBackgroundTaskInvalid;
static bool btstackInitialized;
static bool btstackOpened;
static btstack_packet_handler_t btstackHandler;
static unsigned hciState;

void btpad_queue_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    if (btstackOpened && btstackHandler && hciState == HCI_STATE_WORKING)
    {
        btstackHandler(packet_type, channel, packet, size);
    }

    if (packet_type != HCI_EVENT_PACKET)
    {
        return;
    }

    // HCI_EVENT_PACKET
    switch (packet[0])
    {
        case BTSTACK_EVENT_STATE:
        {        
            hciState = packet[2];

            if (packet[2] == HCI_STATE_WORKING && btstackHandler)
            {
                btstackHandler(packet_type, channel, packet, size);
            } 
            else if (packet[2] == HCI_STATE_HALTING && btstackOpened)
            {
                bt_close();
                
                if (btstackTaskID != UIBackgroundTaskInvalid)
                {
                    [[UIApplication sharedApplication] endBackgroundTask:btstackTaskID];
                    btstackTaskID = UIBackgroundTaskInvalid;                    
                }
                
                btstackOpened = false;                
            }
        }
        return;

        case HCI_EVENT_COMMAND_STATUS:      btpad_queue_run(packet[3]); return;
        case HCI_EVENT_COMMAND_COMPLETE:    btpad_queue_run(packet[2]); return;
    }
}

bool btpad_connect(btstack_packet_handler_t handler)
{
    if (!btstackInitialized)
    {
        run_loop_init(RUN_LOOP_COCOA);
        btstackInitialized = true;
    }

    if (!btstackOpened)
    {
        btpad_queue_reset();

        if (bt_open())
        {
            return false;
        }
        
        btstackOpened = true;
        btstackHandler = handler;
        
        bt_register_packet_handler(btpad_queue_handler);
        btpad_queue_btstack_set_power_mode(HCI_POWER_ON);
        return true;
    }

    return true;
}

void btpad_disconnect()
{
    // We want to keep running until BTstack has halted.
    if (btstackOpened)
    {
        btstackTaskID = [[UIApplication sharedApplication] beginBackgroundTaskWithExpirationHandler:^{
            [[UIApplication sharedApplication] endBackgroundTask:btstackTaskID];
            btstackTaskID = UIBackgroundTaskInvalid;
        }];

        btpad_queue_btstack_set_power_mode(HCI_POWER_OFF);
    }
}

