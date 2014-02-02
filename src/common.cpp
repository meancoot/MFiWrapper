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

#include "common.h"

namespace MFiWrapperCommon {

Connection::Connection(int aDescriptor) :
    Descriptor(aDescriptor),
    Socket(0),
    Source(0),
    Data((uint8_t*)&Packet),
    Position(0)
{
    // Make socket non-blocking
    fcntl(aDescriptor, F_SETFL, fcntl(aDescriptor, F_GETFL, 0) | O_NONBLOCK);

    //
    memset(&Packet, 0, sizeof(Packet));

    CFSocketContext ctx = { 0, this, 0, 0, 0 };
    Socket = CFSocketCreateWithNative(0, Descriptor, kCFSocketReadCallBack,
                                      Callback, &ctx);
    Source = CFSocketCreateRunLoopSource(0, Socket, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), Source, kCFRunLoopCommonModes);
}

Connection::~Connection()
{
    // TODO
    
    CFRunLoopSourceInvalidate(Source);
    CFRelease(Source);
    CFRelease(Socket);
}

void Connection::SendConnect(uint32_t aHandle, const MFiWConnectPacket* aData)
{
    MFiWDataPacket pkt;
    pkt.Size = MFiWPacketConnectSize;
    pkt.Type = MFiWPacketConnect;
    pkt.Handle = aHandle;
    pkt.Connect = *aData;
    write(Descriptor, &pkt, pkt.Size);
}

void Connection::SendDisconnect(uint32_t aHandle)
{
    SendGenericPacket(MFiWPacketDisconnect, aHandle);
}

void Connection::SendInputState(uint32_t aHandle, const MFiWInputStatePacket* aData)
{
    MFiWDataPacket pkt;
    pkt.Size = MFiWPacketInputStateSize;
    pkt.Type = MFiWPacketInputState;
    pkt.Handle = aHandle;
    pkt.State = *aData;
    write(Descriptor, &pkt, pkt.Size);
}

void Connection::SendStartDiscovery()
{
    SendGenericPacket(MFiWPacketStartDiscovery);
}

void Connection::SendStopDiscovery()
{
    SendGenericPacket(MFiWPacketStopDiscovery);
}

void Connection::SendSetPlayerIndex(uint32_t aHandle, int32_t aIndex)
{
    MFiWDataPacket pkt;
    pkt.Size = MFiWPacketSetPlayerIndexSize;
    pkt.Type = MFiWPacketSetPlayerIndex;
    pkt.Handle = aHandle;
    pkt.PlayerIndex.Value = aIndex;
    write(Descriptor, &pkt, pkt.Size);
}

void Connection::SendPausePressed(uint32_t aHandle)
{
    SendGenericPacket(MFiWPacketPausePressed, aHandle);
}

void Connection::SendGenericPacket(MFiWPacketType aType, uint32_t aHandle)
{
    MFiWDataPacket pkt;
    pkt.Size = MFiWPacketGenericSize;
    pkt.Type = aType;
    pkt.Handle = aHandle;
    write(Descriptor, &pkt, pkt.Size);
}

bool Connection::Read()
{
    unsigned targetSize = (Position < 4) ? 4 : Packet.Size;

    ssize_t result = read(Descriptor, &Data[Position], targetSize - Position);

    if (result <= 0)
        return false;
                          
    Position += result;
    assert(Position <= targetSize);
    
    return true;
}

void Connection::Parse()
{
    while (Read())
    {
        if (Position >= Packet.Size)
        {
            HandlePacket(&Packet);

            Position = 0;
            memset(&Packet, 0, sizeof(Packet));
        }
    }
}

void Connection::Callback(CFSocketRef s, CFSocketCallBackType callbackType,
                          CFDataRef address, const void *data, void *info)
{
    using namespace MFiWrapperCommon;

    Connection* connection = (Connection*)info;    
    connection->Parse();
}

}