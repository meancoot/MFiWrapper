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
#include "log.h"
#include "MFiWrapper.h"

extern int HACKStart();
namespace MFiWrapperFrontend {

NSMutableArray* controllers;
MFiWrapperCommon::Logger log("Frontend");
GCControllerDiscoveryCompleteHandler discoveryCompletionHandler;
bool discoveryActive;

class FrontendConnection : public MFiWrapperCommon::Connection
{
public:
    FrontendConnection(int aDescriptor) : Connection(aDescriptor) { };    

    void HandlePacket(const MFiWDataPacket* aPacket)
    {
        log.Verbose("Received packet (Type: %u, Size: %u, Handle: %u)",
                     aPacket->Type, aPacket->Size, aPacket->Handle);

        switch(aPacket->Type)
        {
            case MFiWPacketConnect:      HandlePacketConnect(aPacket);      return;
            case MFiWPacketDisconnect:   HandlePacketDisconnect(aPacket);   return;
            case MFiWPacketInputState:   HandlePacketInputState(aPacket);   return;
            case MFiWPacketPausePressed: HandlePacketPausePressed(aPacket); return;
            default:
                log.Warning("Unknown packet "
                            "(Type: %u, Size: %u, Handle: %u)",
                            aPacket->Type, aPacket->Size, aPacket->Handle);
                return;
        }        
    }
};

FrontendConnection* connection;

static int32_t FindControllerByHandle(uint32_t aHandle)
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

void Startup()
{
    if (!controllers)
    {
        log.Verbose("Creating controllers array.");
        controllers = [[NSMutableArray array] retain];
    }

#ifndef USE_ICADE
    if (!connection)
    {
        log.Verbose("Connecting to backend.");
        connection = new FrontendConnection(HACKStart());
    }
#endif
}

NSArray* GetControllers()
{
    Startup();
    return controllers;
}

void StartWirelessControllerDiscovery(GCControllerDiscoveryCompleteHandler aHandler)
{
    Startup();
 
    // This gets copied even if discovery is already running; the replaced
    // handler will never be called in this scenario. (Check GCController.h in
    // GameController.framework for reference).
    [discoveryCompletionHandler release];
    discoveryCompletionHandler = [aHandler copy]; 
        
    log.Verbose("Starting wireless controller discovery.");
    if (connection && !discoveryActive)
        connection->SendStartDiscovery();
    discoveryActive = true;
}

void StopWirelessControllerDiscovery()
{
    Startup();
    
    log.Verbose("Stopping wireless controller discovery.");

    if (connection && discoveryActive)
        connection->SendStopDiscovery();
    discoveryActive = false;

    if (discoveryCompletionHandler)
    {
        // discoveryCompletionHandler is consumed here to prevent issues if
        // the block it points to makes a call to {Start|Stop}WirelessControllerDiscovery.
        // Without this a double release or infinite recursion are possible.
        GCControllerDiscoveryCompleteHandler gch = discoveryCompletionHandler;
        discoveryCompletionHandler = 0;
        gch();
        [gch release];
    }
}

void SetControllerIndex(uint32_t aHandle, int32_t aIndex)
{
    log.Verbose("Setting controller player index (Handle: %u, Index: %d)",
                 aHandle, aIndex);

    if (connection && aHandle != MFiWLocalHandle)
        connection->SendSetPlayerIndex(aHandle, aIndex);
}

void HandlePacketConnect(const MFiWDataPacket* aPacket)
{
    log.Verbose("New controller connected (Handle: %u, Vendor: %s "
                 "Buttons: %04X Analog: %04X",
                 aPacket->Handle, aPacket->Connect.VendorName,
                 aPacket->Connect.PresentControls, aPacket->Connect.AnalogControls);

    GCController* tweak = [GCControllerTweak controllerForHandle:aPacket->Handle data:aPacket->Connect];
    [controllers addObject:tweak];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification"
                                          object:tweak];
}

void HandlePacketDisconnect(const MFiWDataPacket* aPacket)
{
    log.Verbose("Controller disconnected (Handle: %u)", aPacket->Handle);

    int32_t idx = aPacket->Handle ? FindControllerByHandle(aPacket->Handle) : -1;
    GCController* tweak = (idx >= 0) ? controllers[idx] : nil;

    [tweak retain];
    [controllers removeObjectAtIndex:idx];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification"
                                          object:tweak];
    [tweak release];
}

void HandlePacketInputState(const MFiWDataPacket* aPacket)
{
    log.Verbose("Controller Input State Updated (Handle: %u)", aPacket->Handle);

    int32_t idx = aPacket->Handle ? FindControllerByHandle(aPacket->Handle) : -1;
    GCController* tweak = (idx >= 0) ? controllers[idx] : nil;
    [tweak tweakUpdateButtons:&aPacket->State];
}

void HandlePacketPausePressed(const MFiWDataPacket* aPacket)
{
    log.Verbose("Controller Pause Pressed (Handle: %u)", aPacket->Handle);

    int32_t idx = aPacket->Handle ? FindControllerByHandle(aPacket->Handle) : -1;
    GCController* tweak = (idx >= 0) ? controllers[idx] : nil;

    if (tweak.controllerPausedHandler)
        tweak.controllerPausedHandler(tweak);
}

}
