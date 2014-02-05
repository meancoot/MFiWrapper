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

#include "frontend.h"
#include "MFiWrapper.h"

%hook NSNotificationCenter

- (id)addObserverForName:(NSString *)name object:(id)obj queue:(NSOperationQueue *)queue usingBlock:(void (^)(NSNotification *))block
{
    id result = %orig;
    
    if ([name isEqualToString:@"GCControllerDidConnectNotification"])
        MFiWrapperFrontend::GetControllers();

    return result;
}

- (void)addObserver:(id)notificationObserver selector:(SEL)notificationSelector name:(NSString *)notificationName object:(id)notificationSender
{
    %orig;
    
    if ([notificationName isEqualToString:@"GCControllerDidConnectNotification"])
        MFiWrapperFrontend::GetControllers();
}

%end

%hook GCController

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

%end

// iOS7 iCade Support

@interface UIEvent(iOS7Keyboard)
@property(readonly, nonatomic) long long _keyCode;
@property(readonly, nonatomic) BOOL _isKeyDown;
- (void*)_hidEvent;
@end

#include "keyboard.h"

%hook UIApplication

- (id)_keyCommandForEvent:(UIEvent*)event
{
#ifdef USE_ICADE
    // This gets called twice with the same timestamp for each keypress.
    static double last_time_stamp;

    if (last_time_stamp != [event timestamp])
    {
        last_time_stamp = [event timestamp];
        
        // If the _hidEvent is null, [event _keyCode] will crash.
        // (This happens with the on screen keyboard.)
        if ([event _hidEvent])
        {
            MFiWrapperFrontend::Keyboard::Event([event _isKeyDown], [event _keyCode]);
        }
    }
#endif

    return %orig;
}

%end

