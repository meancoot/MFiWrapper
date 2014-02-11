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

#include <stdio.h>
#include <assert.h>
#include <set>

#include "HIDManager.h"
#include "HIDPad.h"
#include "btstack.h"
#include "log.h"

namespace HIDManager
{
static MFiWrapperCommon::Logger log("btstack");
std::set<Connection*> Connections;

void BTstackPacketHandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);


class Connection
{
public:
    bool connected;

    uint16_t handle;
    bd_addr_t address;

    HIDPad::Interface* hidpad;
    uint16_t channels[2]; //0: Control, 1: Interrupt

    Connection(uint16_t aHandle, bd_addr_t aAddress) :
        connected(false), handle(aHandle), hidpad(0)
    {
        memcpy(address, aAddress, sizeof(address));
        memset(channels, 0, sizeof(channels));
        Connections.insert(this);
    }

    ~Connection()
    {        
        if (handle)
            btpad_queue_hci_disconnect(handle, 0x15);
        Connections.erase(this);
    }    
        
    bool Equals(uint16_t aHandle, bd_addr_t aAddress)
    {
        // Only one of the criteria needs to match.
        return (aHandle && (aHandle == handle)) ||
               (aAddress && (BD_ADDR_CMP(aAddress, address) == 0));
    }
};
    
void StartUp()
{
    run_loop_init(RUN_LOOP_COCOA);
    bt_open();
    bt_register_packet_handler(BTstackPacketHandler);
    bt_send_cmd(&btstack_set_power_mode, HCI_POWER_ON);
}

void ShutDown()
{

}

void SetReport(Connection* aConnection, bool aFeature, uint8_t aID, uint8_t* aData, uint16_t aSize)
{
    bt_send_l2cap(aConnection->channels[0], aData, aSize);
}

void GetReport(Connection* aConnection, bool aFeature, uint8_t aID, uint8_t* aData, uint16_t aSize)
{
    uint8_t data[4] = { 0, aID };
    data[0] = 0x40 | (aFeature ? 3 : 2);
    data[2] = aSize & 0xFF;
    data[3] = (aSize >> 8) & 0xFF;
    bt_send_l2cap(aConnection->channels[0], data, sizeof(data));
}

//

Connection* FindConnection(uint16_t handle, bd_addr_t address)
{
    for (std::set<Connection*>::iterator i = Connections.begin(); i != Connections.end(); i ++)
        if ((*i)->Equals(handle, address))
            return *i;

    return 0;
}

void CloseAllConnections()
{
    while (Connections.size())
        delete *Connections.begin();
}    
    
void BTstackPacketHandler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    bd_addr_t event_addr;

    if (packet_type == L2CAP_DATA_PACKET)
    {
        for (auto i = Connections.begin(); i != Connections.end(); i ++)
        {
            if ((*i)->connected && ((*i)->channels[0] == channel || (*i)->channels[1] == channel))
                (*i)->hidpad->HandlePacket(packet, size);
        }
        
        return;
    }
    else if (packet_type != HCI_EVENT_PACKET)
    {
        log.Warning("Unknown packet type (%02X)", packet_type);
        return;
    }

    // HCI_EVENT_PACKET
    switch (packet[0])
    {
        case BTSTACK_EVENT_STATE:
        {
            log.Verbose("BTstack: HCI State %d\n", packet[2]);
 
            switch (packet[2])
            {                
                case HCI_STATE_WORKING:
                    btpad_queue_reset();

                    bt_send_cmd(&l2cap_register_service, PSM_HID_CONTROL, 672);  // TODO: Where did I get 672 for mtu?
                    bt_send_cmd(&l2cap_register_service, PSM_HID_INTERRUPT, 672);
       
                    btpad_queue_run(1);
                    return;
          
                case HCI_STATE_HALTING:
                    CloseAllConnections();
                    return;                  
            }
        }
        return;

        case HCI_EVENT_COMMAND_STATUS:      btpad_queue_run(packet[3]); return;
        case HCI_EVENT_COMMAND_COMPLETE:    btpad_queue_run(packet[2]); return;

        case HCI_EVENT_CONNECTION_COMPLETE:
        {
            if (packet[2])
            {
                log.Error("Got failed 'Connection Complete' event ",
                          "(Status: %02X)\n", packet[2]);
                return;
            }
            
            bt_flip_addr(event_addr, &packet[5]);        
            new Connection(READ_BT_16(packet, 3), event_addr);
            
            // Needed for WiiMotes
            btpad_queue_hci_switch_role(event_addr, 0);
        }
        return;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
        {
            if (packet[2])
            {
                log.Error("Got failed 'Disconnection Complete' event "
                          "(Status: %02X)\n", packet[2]);
                return;
            }
        
            Connection* connection = FindConnection(READ_BT_16(packet, 3), 0);
            if (connection)
            {
                connection->handle = 0;
                delete connection;
            }
        }
        return;

        case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
        {
            bt_flip_addr(event_addr, &packet[3]);
    
            Connection* connection = FindConnection(0, event_addr);

            if (!connection)
            {
                log.Verbose("BTpad: Got unexpected remote name, ignoring\n");
                return;
            }

            log.Verbose("BTpad: Got %.200s\n", (char*)&packet[9]);
    
            connection->hidpad = HIDPad::Connect((char*)packet + 9, connection);
            connection->connected = true;
        }
        return;

        // L2CAP packets; these are specific to BTstack
        case L2CAP_EVENT_SERVICE_REGISTERED:
        {
            if (packet[2])
            {
                log.Error("Got failed 'Service Registered' event (PSM: %02X, "
                          "Status: %02X)\n", READ_BT_16(packet, 3), packet[2]);
            }
        }
        return;

        case L2CAP_EVENT_INCOMING_CONNECTION:
        {
            // Just accept it
            bt_send_cmd(&l2cap_accept_connection, READ_BT_16(packet, 12));
        }
        return;
        
        case L2CAP_EVENT_CHANNEL_OPENED:
        {
            bt_flip_addr(event_addr, &packet[3]);
            const uint16_t handle = READ_BT_16(packet, 9);
            const uint16_t psm = READ_BT_16(packet, 11);
            const uint16_t channel_id = READ_BT_16(packet, 13);

            Connection* connection = FindConnection(handle, event_addr);

            if (packet[2] || !connection || (psm != PSM_HID_CONTROL && psm != PSM_HID_INTERRUPT))
            {
                log.Error("Got bad L2CAP 'Channel Opened' event (Status: %02X, "
                          "PSM: %02X", packet[2], psm);
                return;
            }

            log.Notice("BTpad: L2CAP channel opened: (PSM: %02X)\n", psm);
            connection->channels[(psm == PSM_HID_CONTROL) ? 0 : 1] = channel_id;

            if (connection->channels[0] && connection->channels[1])
            {
                log.Verbose("BTpad: Got both L2CAP channels, requesting name\n");
                btpad_queue_hci_remote_name_request(connection->address, 0, 0, 0);
            }
        }
        return;        
    }
}

}
