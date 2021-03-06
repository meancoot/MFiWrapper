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
#import <objc/runtime.h>

#include "frontend.h"
#include "MFiWrapper.h"

@interface GCControllerHook : NSObject @end
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

+ (void)startWirelessControllerDiscoveryWithCompletionHandler:(GCControllerDiscoveryCompleteHandler)completionHandler
{
    MFiWrapperFrontend::StartWirelessControllerDiscovery(completionHandler);
}

+ (void)stopWirelessControllerDiscovery
{
    MFiWrapperFrontend::StopWirelessControllerDiscovery();
}

+ (NSArray *)controllers
{
    return MFiWrapperFrontend::GetControllers();
}

@end
