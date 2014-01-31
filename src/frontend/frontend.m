#import <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "frontend.h"
#include "MFiWrapper.h"

static void HandleSocketEvent(CFSocketRef s, CFSocketCallBackType callbackType,
                              CFDataRef address, const void *data, void *info);
static void AttachController(const DataPacket* aData);
static void DetachController(const DataPacket* aData);
static void ControllerState(const DataPacket* aData);


@interface MFiSocket : NSObject
@property (nonatomic, readonly) int descriptor;
@property (nonatomic, readonly) CFSocketRef socket;
@property (nonatomic, readonly) CFRunLoopSourceRef source;
@end

@implementation MFiSocket

- (void)dealloc
{
    CFRelease(_socket);
    CFRelease(_source);
    // close(descriptor);
    
    [super dealloc];
}

- (id)initWithDescriptor:(int)fd
{
    if ((self = [super init]))
    {
        _descriptor = fd;
        _socket = CFSocketCreateWithNative(0, fd, kCFSocketReadCallBack, HandleSocketEvent, 0);
        _source = CFSocketCreateRunLoopSource(0, _socket, 0);
        CFRunLoopAddSource(CFRunLoopGetMain(), _source, kCFRunLoopCommonModes);
    }
    
    return self;
}

@end

@interface MFiDataPacket : NSObject
@property (nonatomic, retain) MFiSocket* socket;
@property (nonatomic) DataPacket packet;
@property (nonatomic) uint8_t* packetData;
@property (nonatomic) uint32_t packetPosition;
@end

@implementation MFiDataPacket

- (void)dealloc
{
    [_socket release];
    [super dealloc];
}

- (id)initWithSocket:(MFiSocket*)socket
{
    if ((self = [super init]))
    {
        self.socket = socket;
        _packetData = (uint8_t*)&_packet;
    }
    
    return self;
}

- (BOOL)read
{
    unsigned targetSize = (self.packetPosition < 4) ? 4 : self.packet.Size;

    ssize_t result = read(self.socket.descriptor, &self.packetData[self.packetPosition],
                          targetSize - self.packetPosition);
                          
    if (result < 0)
        return NO;
                          
    self.packetPosition += result;
    assert(self.packetPosition <= targetSize);
    
    return (result > 0) ? YES : NO;
}

- (void)parse
{
    while ([self read])
    {
        if (self.packetPosition == self.packet.Size)
        {
            switch(self.packet.Type)
            {
                case PKT_OPEN:  AttachController(&_packet); break;
                case PKT_CLOSE: DetachController(&_packet); break;
                case PKT_STATE: ControllerState(&_packet);  break;
            }
        
            self.packetPosition = 0;
            memset(&_packet, 0, sizeof(DataPacket));
        }
    }
}

@end

//
extern int HACKStart();

static MFiSocket* dataSocket;
static MFiDataPacket* dataPacket;
static NSMutableArray* controllers;

static void HandleSocketEvent(CFSocketRef s, CFSocketCallBackType callbackType,
                              CFDataRef address, const void *data, void *info)
{
    [dataPacket parse];
}

static void AttachController(const DataPacket* aData)
{
    GCControllerTweak* tweak = [GCControllerTweak controllerForHandle:aData->Handle data:aData->Open];
    [controllers addObject:tweak];
    [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidConnectNotification" object:tweak];        
}

static void DetachController(const DataPacket* aData)
{
    for (unsigned i = 0; i < [controllers count]; i ++)
    {
        GCController* tweak = controllers[i];

        if (tweak.tweakHandle == aData->Handle)
        {
            [tweak retain];
            [controllers removeObjectAtIndex:i];
            [[NSNotificationCenter defaultCenter] postNotificationName:@"GCControllerDidDisconnectNotification" object:tweak];
            [tweak release];
            return;
        }
    }
}

static void ControllerState(const DataPacket* aData)
{
    for (unsigned i = 0; i < [controllers count]; i ++)
    {
        GCController* tweak = controllers[i];

        if (tweak.tweakHandle == aData->Handle)
            [tweak tweakUpdateButtons:aData->State.Data];
    }
}

void Startup()
{
    if (!controllers)
        controllers = [[NSMutableArray array] retain];

    if (!dataSocket)
        dataSocket = [[MFiSocket alloc] initWithDescriptor:HACKStart()];
        
    if (!dataPacket)
        dataPacket = [[MFiDataPacket alloc] initWithSocket:dataSocket];
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

