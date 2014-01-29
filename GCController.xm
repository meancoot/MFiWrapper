#include <dispatch/dispatch.h>
#include "MFiWrapper.h"
#include "HIDManager.h"

// This array should not be modified on the local thread
static NSMutableArray* controllers;

void attach_tweak_controller(HIDPad::Interface* aInterface)
{
    dispatch_sync(dispatch_get_main_queue(), ^{
        GCControllerTweak* tweak = [GCControllerTweak controllerForHIDPad:aInterface];
        [controllers addObject:tweak];
        [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification" object:tweak];        
    });
}

void detach_tweak_controller(HIDPad::Interface* aInterface)
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

static void do_startup()
{
    if (!controllers)
    {
        controllers = [[NSMutableArray array] retain];
        HIDManager::StartUp();
    }
}

%hook GCController

+ (void)startWirelessControllerDiscoveryWithCompletionHandler:(void (^)(void))completionHandler
{
    do_startup();
    HIDManager::StartDeviceProbe();
}

+ (void)stopWirelessControllerDiscovery
{
    do_startup();
    HIDManager::StopDeviceProbe();
}

+ (NSArray *)controllers
{
    do_startup();
    return controllers;
}

%end

