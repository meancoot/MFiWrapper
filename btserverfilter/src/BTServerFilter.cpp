#include <stdlib.h>
#include <substrate.h>
#include <CoreFoundation/CoreFoundation.h>

#include <vector>
#include <map>
#include <string>

#include "BTAddress.h"
#include "PacketLogger.h"
static PacketLogger* pktLog;

#define HOOKDEF(T, X, ...)  static T (*original_##X)(__VA_ARGS__); static T new_##X(__VA_ARGS__)
#define HOOK(X)             MSHookFunction((void*)&X, (void*)&new_##X, (void**)&original_##X)

struct ReadContext;

// Maps Bluetooth Address to corresponding Device Names.
// Filled by Remote Name Request Complete events, and checked
// by PIN Code Request Reply commands to send the proper PIN
// for Wii Remotes.
static std::map<BTAddress, std::string> deviceNames;

// The reversed address of the local Bluetooth radio.
// Used as the PIN for Wii Remotes.
static uint8_t localAddress[6];

// List of filters, indexed by HCI event ID.
// The return value is ignored.
typedef bool (*ReadFilter)(ReadContext& aContext);
static ReadFilter ReadFilters[0x100];

// List of filters, indexed by HCI command opcode.
// The filter is called before the buffer is written to the Bluetooth device.
// If the filter returns a value greater than zero the data will not written,
// in which case the filter should call original_write to send it's modified data.
typedef ssize_t (*WriteFilter)(int aFile, const uint8_t* aBuffer, size_t aSize);
static WriteFilter WriteFilters[0x10000];

// 
struct ReadContext
{
    uint8_t* data;
    ssize_t dataLength;
    size_t offset;
    
    uint32_t size;
    std::vector<uint8_t> context;
    
    void WriteByte(uint32_t aOffset, uint8_t aValue)
    {
        if (aOffset > context.size())
            return;
    
        context[aOffset] = aValue;
        if (aOffset < offset + size)
        {
            data[aOffset - offset] = aValue;
        }
    }
};

HOOKDEF(ssize_t, write, int fildes, const void* buf, size_t nbyte)
{
    const uint8_t* bb = (const uint8_t*)buf;

    // Packet Logger
    if (pktLog && (bb[0] == 0x01 || bb[0] == 0x02))
    {
        pktLog->LogWrite(bb, nbyte);
    }

    ssize_t writeFilter = 0;

    if (bb[0] == 0x01)
    {
        uint32_t cmdID = (bb[2] << 8) | bb[1];
        writeFilter = WriteFilters[cmdID] ? WriteFilters[cmdID](fildes, bb, nbyte) : 0;
    }

    return (writeFilter < 1) ? original_write(fildes, buf, nbyte) : nbyte;
}

HOOKDEF(ssize_t, read, int fildes, void* buf, size_t nbyte)
{
    static ReadContext ctx;
    
    ctx.dataLength = original_read(fildes, buf, nbyte);

    if (ctx.dataLength < 1)
        return ctx.dataLength;

    ctx.data = (uint8_t*)buf;
    ctx.offset = ctx.context.size();
    ctx.context.resize(ctx.offset + ctx.dataLength);
    memcpy(&ctx.context[ctx.offset], buf, ctx.dataLength);
    ctx.size = ctx.context.size();

    // Get expected packet size
    uint32_t pktSize = 0;
    if (ctx.context[0] == 0x04)
    {
        pktSize = ctx.context[2] + 3;
    }
    else if (ctx.context[0] == 0x02)
    {
        pktSize = (ctx.context[3] | (ctx.context[4] << 8)) + 5;
    }
    
    // FILTERS
    if (ctx.context[0] == 0x04 && ReadFilters[ctx.context[1]])
    {
        ReadFilters[ctx.context[1]](ctx);
    }

    // Clean up and log    
    if (ctx.size >= pktSize)
    {
        if (pktLog && pktSize)
        {
            pktLog->LogRead(&ctx.context[0], pktSize);
        }
        
        ctx.context.clear();
    }

    return ctx.dataLength;
}

// This filter changes the role of the connection request command to master.
// TODO: Force this only for Wii Remotes.
ssize_t FilterAcceptConnectionRequestCommand(int aFile, const uint8_t* aBuffer, size_t aSize)
{
    if (aSize == 11)
    {
        uint8_t newPacket[11];
        memcpy(newPacket, aBuffer, aSize);
        newPacket[10] = 0;
    
        return original_write(aFile, newPacket, 11);
    }
    
    return 0;
}

// If the target Bluetooth Device's name has the text "Nintendo RVL-CNT-01" in
// it this filter will replace the PIN with the reversed address of the iOS Device's
// Bluetooth radio.
ssize_t FilterPINCodeRequestReplyCommand(int aFile, const uint8_t* aBuffer, size_t aSize)
{
    if (aSize == 27)
    {
        auto name = deviceNames.find(&aBuffer[4]);
        
        if (name == deviceNames.end() || name->second.find("Nintendo RVL-CNT-01", 0) == std::string::npos)
        {
            return 0;
        }
    
        uint8_t newPacket[27];
        memcpy(newPacket, aBuffer, aSize);
        newPacket[10] = 6;
        memcpy(&newPacket[11], localAddress, 6);
        
        return original_write(aFile, newPacket, 27);
    }
    
    return 0;
}

// This functions stores a map of Bluetooth Addresses to Device Names.
bool FilterRemoteNameRequestCompleteEvent(ReadContext& aContext)
{
    if (aContext.size < 255)
    {
        return false;
    }
    
    deviceNames[&aContext.context[4]] = (const char*)&aContext.context[10];
    return false;
}

// This filter waits for a completed Read Local Address command to complete
// then copies the address and unmaps itself (to avoid checking every Command
// Complete event for the local address).
bool FilterCommandCompleteEvent(ReadContext& aContext)
{
    if (aContext.size < 13 || aContext.context[4] != 0x09 || aContext.context[5] != 0x10)
    {
        return false;
    }
    
    memcpy(localAddress, &aContext.context[7], 6);
    
    // Don't capture any more command completes
    ReadFilters[0x0E] = 0;
    return false;    
}

// This filter patches inquiry result events, but only those which list 
// a single device. If the major class of device is Peripheral the minor
// class will be set to keyboard.
// NOTE: This filter is used for both Inquiry Result with RSSI and Extended
//       Inquiry Result events.
bool FilterInquiryResultEventGeneral(ReadContext& aContext)
{
    // We won't patch packets with more than one response
    if (aContext.size < 0x10 || aContext.context[3] != 1)
    {
        return false;
    }

    if ((aContext.context[13] & 0x1F) == 0x05)   // Peripheral 
    {
        aContext.WriteByte(12, 0x40);            // Set to Keyboard            
    }

    return false;    
}

__attribute__ ((constructor)) static void entry(void)
{
//    pktLog = new PacketLogger("/tmp/MFiWrapper.pklg");

    WriteFilters[0x0409] = FilterAcceptConnectionRequestCommand;
    WriteFilters[0x040D] = FilterPINCodeRequestReplyCommand;

    ReadFilters[0x07] = FilterRemoteNameRequestCompleteEvent;
    ReadFilters[0x0E] = FilterCommandCompleteEvent;
    ReadFilters[0x22] = FilterInquiryResultEventGeneral;
    ReadFilters[0x2F] = FilterInquiryResultEventGeneral;    

    HOOK(write);
    HOOK(read);
}
