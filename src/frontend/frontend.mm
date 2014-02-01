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

#include "frontend.h"
#include "common.h"
#include "MFiWrapper.h"

extern int HACKStart();
namespace MFiWrapperFrontend {

NSMutableArray* controllers;

class FrontendConnection : public MFiWrapperCommon::Connection
{
    public:
        FrontendConnection(int aDescriptor) : MFiWrapperCommon::Connection(aDescriptor) { };
        
        int32_t FindControllerByHandle(uint32_t aHandle)
        {
            for (int32_t i = 0; i < [controllers count]; i ++)
            {
                GCController* tweak = controllers[i];
                if (tweak.tweakHandle == aHandle)
                {
                    return i;
                }
            }
            
            return -1;
        }
        
        void AttachController(const MFiWDataPacket* aData)
        {
            GCControllerTweak* tweak = [GCControllerTweak controllerForHandle:aData->Handle data:aData->Connect];
            [controllers addObject:tweak];
            [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification" object:tweak];        
        }

        void DetachController(const MFiWDataPacket* aData)
        {
            int32_t idx = FindControllerByHandle(aData->Handle);
            
            if (idx < 0)
                return;
                
            GCController* tweak = controllers[idx];
            [tweak retain];
            [controllers removeObjectAtIndex:idx];
            [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification" object:tweak];
            [tweak release];
        }
        
        void ControllerState(const MFiWDataPacket* aData)
        {
            int32_t idx = FindControllerByHandle(aData->Handle);
            
            if (idx < 0)
                return;

            GCController* tweak = controllers[idx];
            [tweak tweakUpdateButtons:&aData->State];
        }
        
        void HandlePacket(const MFiWDataPacket* aPacket)
        {
            switch(aPacket->Type)
            {
                case MFiWPacketConnect:     AttachController(aPacket); break;
                case MFiWPacketDisconnect:  DetachController(aPacket); break;
                case MFiWPacketInputState:  ControllerState (aPacket); break;
                default: printf("Unknown packet sent to frontend: %d\n", aPacket->Type); break;
            }        
        }
};

FrontendConnection* connection;

void Startup()
{
    if (!controllers)
        controllers = [[NSMutableArray array] retain];

    if (!connection)
        connection = new FrontendConnection(HACKStart());
}

NSArray* GetControllers()
{
    Startup();
    return controllers;
}

void StartWirelessControllerDiscovery()
{
    Startup();
    connection->SendStartDiscovery();
}

void StopWirelessControllerDiscovery()
{
    Startup();
    connection->SendStopDiscovery();    
}

void SetControllerIndex(uint32_t aHandle, int32_t aIndex)
{
    connection->SendSetPlayerIndex(aHandle, aIndex);
}

}
