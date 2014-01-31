#import <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "frontend.h"
#include "MFiWrapper.h"

extern int HACKStart();
namespace MFiWrapperFrontend {

int _socketFD;
CFSocketRef _socket;
CFRunLoopSourceRef _socketSource;
NSMutableArray* controllers;

void HandleSocketEvent(CFSocketRef s, CFSocketCallBackType callbackType,
                       CFDataRef address, const void *data, void *info)
{
    printf("DATA\n");
}

void Startup()
{
    if (!controllers)
        controllers = [[NSMutableArray array] retain];

    if (!_socket)
    {
        _socketFD = HACKStart();

        _socket = CFSocketCreateWithNative(0, _socketFD, kCFSocketDataCallBack, HandleSocketEvent, 0);
        _socketSource = CFSocketCreateRunLoopSource(0, _socket, 0);
        CFRunLoopAddSource(CFRunLoopGetCurrent(), _socketSource, kCFRunLoopCommonModes);
    }
}

/*
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
        for (unsigned i = 0; i < [controllers count]; i ++)
        {
            GCController* tweak = controllers[i];
    
            if (tweak.tweakHIDPad == aInterface)
            {
                [tweak retain];
                [controllers removeObjectAtIndex:i];
                [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification" object:tweak];
                [tweak release];
                return;
            }
        }
    });
}
*/

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
