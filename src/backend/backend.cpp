#import <CoreFoundation/CoreFoundation.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <map>

#include "HIDManager.h"
#include "HIDPad.h"

#include "backend.h"
#include "protocol.h"
#include "common.h"

namespace MFiWrapperBackend {

static uint32_t nextHandle = 1;
static std::map<HIDPad::Interface*, uint32_t> devices;
static pthread_t thread;
static int sockets[2] = { -1, -1 };

class BackendConnection : MFiWrapperCommon::Connection
{
    public:
        BackendConnection(int aDescriptor) : MFiWrapperCommon::Connection(aDescriptor) { };
        
        HIDPad::Interface* FindDeviceByHandle(uint32_t aHandle)
        {
            for (auto i = devices.begin(); i != devices.end(); i ++)
                if (i->second == aHandle)
                    return i->first;
            return 0;
        }
        
        void HandlePacket(const MFiWDataPacket* aPacket)
        {
            switch(aPacket->Type)
            {
                case MFiWPacketStartDiscovery:
                    HIDManager::StartDeviceProbe();
                    return;
                
                case MFiWPacketStopDiscovery:
                    HIDManager::StopDeviceProbe();
                    return;
                
                case MFiWPacketSetPlayerIndex:
                {
                    HIDPad::Interface* device = FindDeviceByHandle(aPacket->Handle);
                    if (device)
                        device->SetPlayerIndex(aPacket->PlayerIndex.Value);
                    return;
                }
                
                default:
                    printf("Unkown packet set to backend: %d\n", aPacket->Type);
                    return;
            }        
        }
};

BackendConnection* connection;

static void* ManagerThread(void* aUnused)
{
    connection = new BackendConnection(sockets[0]);

    HIDManager::StartUp();    
    CFRunLoopRun();
    HIDManager::ShutDown();
    
    return 0;
}

void AttachController(HIDPad::Interface* aInterface)
{
    if (devices.find(aInterface) == devices.end())
    {
        uint32_t handle = nextHandle ++;
        devices[aInterface] = handle;
        
        MFiWDataPacket pkt;
        pkt.Size = sizeof(MFiWDataPacket);
        pkt.Type = MFiWPacketConnect;
        pkt.Handle = handle;
        strlcpy(pkt.Connect.VendorName, "Test", sizeof(pkt.Connect.VendorName));
        pkt.Connect.PresentControls = 0xFFFFFFFF;
        pkt.Connect.AnalogControls = 0;
        write(sockets[0], &pkt, pkt.Size);
    }
}

void DetachController(HIDPad::Interface* aInterface)
{
    std::map<HIDPad::Interface*, uint32_t>::iterator device;
    if ((device = devices.find(aInterface)) != devices.end())
    {
        uint32_t handle = device->second;
        devices.erase(device);
        
        MFiWDataPacket pkt;
        pkt.Size = sizeof(MFiWDataPacket);
        pkt.Type = MFiWPacketDisconnect;
        pkt.Handle = handle;
        write(sockets[0], &pkt, pkt.Size);
    }
}

void SendControllerState(HIDPad::Interface* aInterface, const float aData[32])
{
    std::map<HIDPad::Interface*, uint32_t>::iterator device;
    if ((device = devices.find(aInterface)) != devices.end())
    {
        uint32_t handle = device->second;
        
        MFiWDataPacket pkt;
        pkt.Size = sizeof(MFiWDataPacket);
        pkt.Type = MFiWPacketInputState;
        pkt.Handle = handle;
        memcpy(pkt.State.Data, aData, sizeof(pkt.State.Data));
        write(sockets[0], &pkt, pkt.Size);
    }
}

}

int HACKStart()
{
    using namespace MFiWrapperBackend;

    if (!thread)
    {
        socketpair(PF_LOCAL, SOCK_STREAM, 0, sockets);
        pthread_create(&thread, 0, ManagerThread, 0);
    }
    
    return sockets[1];
}
