#include <dispatch/dispatch.h>
#include "MFiWrapper.h"
#include "HIDManager.h"

static NSMutableArray* controllers;

void MFiWrapper::Startup()
{
    if (!controllers)
    {
        controllers = [[NSMutableArray array] retain];
        HIDManager::StartUp();
    }
}

void MFiWrapper::AttachController(HIDPad::Interface* aInterface)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        GCControllerTweak* tweak = [GCControllerTweak controllerForHIDPad:aInterface];
        [controllers addObject:tweak];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification" object:tweak];        
    });
}

void MFiWrapper::DetachController(HIDPad::Interface* aInterface)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        for (int i = 0; i != [controllers count]; i ++)
        {
            GCController* tweak = controllers[i];
    
            if (tweak.tweakHIDPad == aInterface)
            {
                [tweak retain];
                [controllers removeObjectAtIndex:i];
                [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification" object:tweak];
                [tweak release];
            }
        }
    });
}

NSArray* MFiWrapper::GetControllers()
{
    Startup();
    return controllers;
}

void MFiWrapper::StartWirelessControllerDiscovery()
{
    Startup();
    HIDManager::StartDeviceProbe();
}

void MFiWrapper::StopWirelessControllerDiscovery()
{
    Startup();
    HIDManager::StopDeviceProbe();
}
