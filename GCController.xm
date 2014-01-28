#include "MFiWrapper.h"
#include "HIDManager.h"

static NSMutableArray* controllers;

static void init_controllers()
{
    if (!controllers)
        controllers = [[NSMutableArray array] retain];
}

void attach_tweak_controller(HIDPad::Interface* aInterface)
{
    init_controllers();

    GCControllerTweak* tweak = [GCControllerTweak controllerForHIDPad:aInterface];
    [controllers addObject:tweak];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification" object:tweak];
}

void detach_tweak_controller(HIDPad::Interface* aInterface)
{
    init_controllers();

    for (int i = 0; i != [controllers count]; i ++)
    {
        GCControllerTweak* tweak = controllers[i];
    
        if (tweak.tweakHIDPad == aInterface)
        {
            [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification" object:controllers[i]];
            [controllers removeObjectAtIndex:i];
        }
    }
}

%hook GCController

+ (void)startWirelessControllerDiscoveryWithCompletionHandler:(void (^)(void))completionHandler
{
    HIDManager::StartDeviceProbe();
}

+ (void)stopWirelessControllerDiscovery
{
    HIDManager::StopDeviceProbe();
}

+ (NSArray *)controllers
{
    HIDManager::StartUp();
    HIDManager::StartDeviceProbe();

    init_controllers();
    return controllers;
}

%end

