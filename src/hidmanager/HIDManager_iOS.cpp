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

#include <CoreFoundation/CFRunLoop.h>

#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <set>

#include "HIDManager.h"
#include "HIDPad.h"
#include "btstack.h"

namespace HIDManager
{
#ifndef NDEBUG
    #define BTPAD_LOG(...) printf(__VA_ARGS__)
    #define ASSERT_THREAD assert(pthread_equal(btstackThread, pthread_self()))
#else
    #define BTPAD_LOG(...)
    #define ASSERT_THREAD
#endif

    bool inquiry_off;
    bool inquiry_running;

    pthread_t btstackThread;
    CFRunLoopRef btstackRunLoop;

    enum { BTPAD_EMPTY, BTPAD_CONNECTING, BTPAD_CONNECTED };

    std::set<Connection*> Connections;

    void* BTstackThreadFunction(void* unused);
    void BTstackPacketHandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


    class Connection
    {   public:
        uint32_t state;
    
        uint16_t handle;
        bd_addr_t address;
        bool hasAddress;

        HIDPad::Interface* hidpad;
        uint16_t channels[2]; //0: Control, 1: Interrupt
    
        Connection() : state(BTPAD_EMPTY), handle(0), hasAddress(false),
                       hidpad(0)
        {
            ASSERT_THREAD;        
            memset(address, 0, sizeof(address));
            memset(channels, 0, sizeof(channels));
            Connections.insert(this);
        }
    
        ~Connection()
        {
            ASSERT_THREAD;        
            if (handle)
                btpad_queue_hci_disconnect(handle, 0x15);
            Connections.erase(this);
        }    
        
        void SetAddress(bd_addr_t aAddress)
        {
            ASSERT_THREAD;
            memcpy(address, aAddress, sizeof(bd_addr_t));
            hasAddress = true;
        }
        
        bool Equals(uint16_t aHandle, bd_addr_t aAddress)
        {
            ASSERT_THREAD;
            if (!handle && !address)
                return false;
            else if (aHandle && handle && handle != aHandle)
                return false;
            else if (aAddress && hasAddress && (BD_ADDR_CMP(address, aAddress)))
                return false;
            else
                return true; 
        }
    };
        
    void StartUp()
    {
        if (!btstackThread)
            pthread_create(&btstackThread, 0, BTstackThreadFunction, 0);  
    }
    
    void ShutDown()
    {

    }
    
    void SendPacket(Connection* aConnection, uint8_t* aData, size_t aSize)
    {
        if (btstackRunLoop == CFRunLoopGetCurrent())
            bt_send_l2cap(aConnection->channels[0], aData, aSize);
        else if (btstackRunLoop)
        {
            // (TODO) THREADING: What if aConnection is deleted before
            //                   the block is run? Maybe the block can
            //                   check if aConnection is present in the
            //                   Connections set before running.
            uint8_t* data = new uint8_t[aSize];
            memcpy(data, aData, aSize);

            CFRunLoopPerformBlock(btstackRunLoop, kCFRunLoopCommonModes, ^{
                bt_send_l2cap(aConnection->channels[0], data, aSize);
                delete[] data;
            });
            CFRunLoopWakeUp(btstackRunLoop);
        }
    }
    
    void StartDeviceProbe()
    {
        if (!btstackRunLoop)
            return;
    
        CFRunLoopPerformBlock(btstackRunLoop, kCFRunLoopCommonModes, ^{
            inquiry_off = false;
            
            if (!inquiry_running)
                btpad_queue_hci_inquiry(HCI_INQUIRY_LAP, 3, 1);
        });
    }
    
    void StopDeviceProbe()
    {
        if (!btstackRunLoop)
            return;
    
        CFRunLoopPerformBlock(btstackRunLoop, kCFRunLoopCommonModes, ^{
            inquiry_off = true;
        });
    }

    //
    
    Connection* FindConnection(uint16_t handle, bd_addr_t address)
    {
        ASSERT_THREAD;
        for (std::set<Connection*>::iterator i = Connections.begin(); i != Connections.end(); i ++)
            if ((*i)->Equals(handle, address))
                return *i;

        return 0;
    }
    
    void CloseAllConnections()
    {
        ASSERT_THREAD;
        while (Connections.size())
            delete *Connections.begin();
    }    
    
    void* BTstackThreadFunction(void* unused)
    {
        ASSERT_THREAD;    
        run_loop_init(RUN_LOOP_COCOA);
        btstackRunLoop = CFRunLoopGetCurrent();        
        bt_open();
        bt_register_packet_handler(BTstackPacketHandler);
        bt_send_cmd(&btstack_set_power_mode, HCI_POWER_ON);
        CFRunLoopRun();
    
        return 0;    
    }
    
    void BTstackPacketHandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
    {
       bd_addr_t event_addr;

       if (packet_type == HCI_EVENT_PACKET)
       {
          switch (packet[0])
          {
             case BTSTACK_EVENT_STATE:
             {
                BTPAD_LOG("BTstack: HCI State %d\n", packet[2]);
         
                switch (packet[2])
                {                  
                   case HCI_STATE_WORKING:
                      btpad_queue_reset();

                      btpad_queue_hci_read_bd_addr();
                      bt_send_cmd(&l2cap_register_service, PSM_HID_CONTROL, 672);  // TODO: Where did I get 672 for mtu?
                      bt_send_cmd(&l2cap_register_service, PSM_HID_INTERRUPT, 672);
                      btpad_queue_hci_inquiry(HCI_INQUIRY_LAP, 3, 1);
               
                      btpad_queue_run(1);
                      break;
                  
                   case HCI_STATE_HALTING:
                      CloseAllConnections();
                      CFRunLoopStop(CFRunLoopGetCurrent());
                      break;                  
                }
             }
             break;

             case HCI_EVENT_COMMAND_STATUS:
             {
                btpad_queue_run(packet[3]);
             }
             break;

             case HCI_EVENT_COMMAND_COMPLETE:
             {
                btpad_queue_run(packet[2]);

                if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr))
                {
                   bt_flip_addr(event_addr, &packet[6]);
                   if (!packet[5]) BTPAD_LOG("BTpad: Local address is %s\n", bd_addr_to_str(event_addr));
                   else            BTPAD_LOG("BTpad: Failed to get local address (Status: %02X)\n", packet[5]);               
                }
             }
             break;

             case HCI_EVENT_INQUIRY_RESULT:
             {
                if (packet[2])
                {
                   bt_flip_addr(event_addr, &packet[3]);

                   Connection* connection = new Connection();
                   if (connection)
                   {
                      BTPAD_LOG("BTpad: Inquiry found device\n");

                      connection->SetAddress(event_addr);
                      connection->state = BTPAD_CONNECTING;

                      bt_send_cmd(&l2cap_create_channel, connection->address, PSM_HID_CONTROL);
                      bt_send_cmd(&l2cap_create_channel, connection->address, PSM_HID_INTERRUPT);
                   }
                }
             }
             break;

             case HCI_EVENT_INQUIRY_COMPLETE:
             {
                // This must be turned off during gameplay as it causes a ton of lag
                inquiry_running = !inquiry_off;

                if (inquiry_running)
                   btpad_queue_hci_inquiry(HCI_INQUIRY_LAP, 3, 1);
             }
             break;

             case L2CAP_EVENT_CHANNEL_OPENED:
             {
                bt_flip_addr(event_addr, &packet[3]);
                const uint16_t handle = READ_BT_16(packet, 9);
                const uint16_t psm = READ_BT_16(packet, 11);
                const uint16_t channel_id = READ_BT_16(packet, 13);

                Connection* connection = FindConnection(handle, event_addr);

                if (!packet[2])
                {
                   if (!connection)
                   {
                      BTPAD_LOG("BTpad: Got L2CAP 'Channel Opened' event for unrecognized device\n");
                      break;
                   }

                   BTPAD_LOG("BTpad: L2CAP channel opened: (PSM: %02X)\n", psm);
                   connection->handle = handle;
            
                   if (psm == PSM_HID_CONTROL)
                      connection->channels[0] = channel_id;
                   else if (psm == PSM_HID_INTERRUPT)
                      connection->channels[1] = channel_id;
                   else
                      BTPAD_LOG("BTpad: Got unknown L2CAP PSM, ignoring (PSM: %02X)\n", psm);

                   if (connection->channels[0] && connection->channels[1])
                   {
                      BTPAD_LOG("BTpad: Got both L2CAP channels, requesting name\n");
                      btpad_queue_hci_remote_name_request(connection->address, 0, 0, 0);
                   }
                }
                else
                   BTPAD_LOG("BTpad: Got failed L2CAP 'Channel Opened' event (PSM: %02X, Status: %02X)\n", psm, packet[2]);
             }
             break;

             case L2CAP_EVENT_INCOMING_CONNECTION:
             {
                bt_flip_addr(event_addr, &packet[2]);
                const uint16_t handle = READ_BT_16(packet, 8);
                const uint32_t channel_id = READ_BT_16(packet, 12);
      
                Connection* connection = FindConnection(handle, event_addr);

                if (!connection)
                {
                   connection = new Connection();
                   if (connection)
                   {
                      BTPAD_LOG("BTpad: Got new incoming connection\n");

                      connection->SetAddress(event_addr);
                      connection->handle = handle;
                      connection->state = BTPAD_CONNECTING;
                   }
                   else break;
                }

                BTPAD_LOG("BTpad: Incoming L2CAP connection (PSM: %02X)\n", READ_BT_16(packet, 10));
                bt_send_cmd(&l2cap_accept_connection, channel_id);
             }
             break;

             case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
             {
                bt_flip_addr(event_addr, &packet[3]);
            
                Connection* connection = FindConnection(0, event_addr);

                if (!connection)
                {
                   BTPAD_LOG("BTpad: Got unexpected remote name, ignoring\n");
                   break;
                }

                BTPAD_LOG("BTpad: Got %.200s\n", (char*)&packet[9]);
            
                connection->hidpad = HIDPad::Connect((char*)packet + 9, connection);
                connection->state = BTPAD_CONNECTED;
             }
             break;

             case HCI_EVENT_PIN_CODE_REQUEST:
             {
                BTPAD_LOG("BTpad: Sending WiiMote PIN\n");

                bt_flip_addr(event_addr, &packet[2]);
                btpad_queue_hci_pin_code_request_reply(event_addr, &packet[2]);
             }
             break;

             case HCI_EVENT_DISCONNECTION_COMPLETE:
             {
                const uint32_t handle = READ_BT_16(packet, 3);

                if (!packet[2])
                {
                   Connection* connection = FindConnection(handle, 0);
                   if (connection)
                   {
                      connection->handle = 0;
                      delete connection;
                   }
                }
                else
                   BTPAD_LOG("BTpad: Got failed 'Disconnection Complete' event (Status: %02X)\n", packet[2]);
             }
             break;

             case L2CAP_EVENT_SERVICE_REGISTERED:
             {
                if (packet[2])
                   BTPAD_LOG("BTpad: Got failed 'Service Registered' event (PSM: %02X, Status: %02X)\n", READ_BT_16(packet, 3), packet[2]);
             }
             break;
          }
       }
       else if (packet_type == L2CAP_DATA_PACKET)
       {
          for (auto i = Connections.begin(); i != Connections.end(); i ++)
          {
             if ((*i)->state == BTPAD_CONNECTED && ((*i)->channels[0] == channel || (*i)->channels[1] == channel))
                (*i)->hidpad->HandlePacket(packet, size);
          }
       }
    }
    
}
