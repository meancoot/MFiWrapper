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

#import <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <map>

#include "HIDManager.h"
#include "HIDPad.h"

#include "backend.h"
#include "protocol.h"
#include "common.h"
#include "log.h"

namespace MFiWrapperBackend {

static uint32_t nextHandle = 1;
static std::map<uint32_t, HIDPad::Interface*> devices;
static pthread_t thread;
static int sockets[2] = { -1, -1 };

static MFiWrapperCommon::Logger log("Backend");

class BackendConnection : public MFiWrapperCommon::Connection
{
    public:
        BackendConnection(int aDescriptor) : MFiWrapperCommon::Connection(aDescriptor) { };
        
        void HandlePacket(const MFiWDataPacket* aPacket)
        {
            log.Verbose("Received packet (Type: %u, Size: %u, Handle: %u)",
                        aPacket->Type, aPacket->Size, aPacket->Handle); 
        
            switch(aPacket->Type)
            {
                case MFiWPacketSetPlayerIndex:
                {
                    log.Verbose("Received player index packet (Handle: %u, Index: %d)",
                                aPacket->Handle, aPacket->PlayerIndex.Value);

                    HIDPad::Interface* device = devices[aPacket->Handle];
                    if (device)
                        device->SetPlayerIndex(aPacket->PlayerIndex.Value);
                    return;
                }
                
                default:
                    log.Warning("Unknown packet "
                                "(Type: %u, Size: %u, Handle: %u)",
                                aPacket->Type, aPacket->Size, aPacket->Handle);
                    return;
            }        
        }
};

BackendConnection* connection;

static void* ManagerThread(void* aUnused)
{
    log.Verbose("Manager thread starting.");

    connection = new BackendConnection(sockets[0]);

    HIDManager::StartUp();
    
    log.Verbose("Manager thread entering run loop.");
    CFRunLoopRun();
    log.Verbose("Manager thread left run loop.");
    
    HIDManager::ShutDown();
    
    return 0;
}

uint32_t AttachController(HIDPad::Interface* aInterface)
{
    uint32_t handle = nextHandle ++;
    devices[handle] = aInterface;

    MFiWConnectPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    strlcpy(pkt.VendorName, aInterface->GetVendorName(), sizeof(pkt.VendorName));
    pkt.PresentControls = aInterface->GetPresentControls();
    pkt.AnalogControls = aInterface->GetAnalogControls();
    connection->SendConnect(handle, &pkt);
    
    return handle;
}

void DetachController(HIDPad::Interface* aInterface)
{
    log.Verbose("Detaching controller (Handle: %u)", aInterface->GetHandle());

    if (devices.erase(aInterface->GetHandle()))
        connection->SendDisconnect(aInterface->GetHandle());
}

void SendControllerState(HIDPad::Interface* aInterface, const MFiWInputStatePacket* aData)
{
    log.Verbose("Sending controller state (Handle: %u)", aInterface->GetHandle());
    connection->SendInputState(aInterface->GetHandle(), aData);
}

void SendPausePressed(HIDPad::Interface* aInterface)
{
    log.Verbose("Sending pause pressed (Handle: %u)", aInterface->GetHandle());
    connection->SendPausePressed(aInterface->GetHandle());
}

}

int HACKStart()
{
    using namespace MFiWrapperBackend;

    if (!thread)
    {
        socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets);
        pthread_create(&thread, 0, ManagerThread, 0);
    }
    
    return sockets[1];
}
