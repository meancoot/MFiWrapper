#import <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "frontend.h"
#include "common.h"
#include "MFiWrapper.h"

extern int HACKStart();
namespace MFiWrapperFrontend {

NSMutableArray* controllers;

class FrontendConnection : MFiWrapperCommon::Connection
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
            [tweak tweakUpdateButtons:aData->State.Data];
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
}

void StopWirelessControllerDiscovery()
{
    Startup();
}

}
