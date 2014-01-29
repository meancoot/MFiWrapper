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

#include <Foundation/Foundation.h>
#include <dispatch/dispatch.h>
#import <objc/runtime.h>

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

@interface GCControllerHook : NSObject
@end

@implementation GCControllerHook

+ (void)load
{
    Class gcOrig = NSClassFromString(@"GCController");
    Class gcHook = NSClassFromString(@"GCControllerHook");
    
    if (!gcOrig)
        return;

    Method orig, hook;
    #define EXCH(x) \
        orig = class_getClassMethod(gcOrig, @selector(x)); \
        hook = class_getClassMethod(gcHook, @selector(x)); \
        method_exchangeImplementations(orig, hook);
    EXCH(startWirelessControllerDiscoveryWithCompletionHandler:);
    EXCH(stopWirelessControllerDiscovery);
    EXCH(controllers);
}

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

@end
