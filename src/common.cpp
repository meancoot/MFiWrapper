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

MFiWrapperCommon::Connection::Connection(int aDescriptor) :
    Descriptor(aDescriptor),
    Socket(0),
    Source(0),
    Data((uint8_t*)&Packet),
    Position(0)
{
    memset(&Packet, 0, sizeof(Packet));
    
    Socket = CFSocketCreateWithNative(0, Descriptor, kCFSocketReadCallBack, 0, 0);
    Source = CFSocketCreateRunLoopSource(0, Socket, 0);
    CFRunLoopAddSource(CFRunLoopGetCurrent(), Source, kCFRunLoopCommonModes);
}

MFiWrapperCommon::Connection::~Connection()
{
    // TODO
    
    CFRunLoopSourceInvalidate(Source);
    CFRelease(Source);
    CFRelease(Socket);
}

bool MFiWrapperCommon::Connection::Read()
{
    unsigned targetSize = (Position < 4) ? 4 : Packet.Size;

    ssize_t result = read(Descriptor, &Data[Position], targetSize - Position);

    if (result <= 0)
        return false;
                          
    Position += result;
    assert(Position <= targetSize);
    
    return true;
}

void MFiWrapperCommon::Connection::Parse()
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
