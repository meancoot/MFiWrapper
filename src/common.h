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

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <stdint.h>
#include "protocol.h"

namespace MFiWrapperCommon {

class Connection
{
    public:
        Connection(int aDescriptor);
        ~Connection();

        void SendConnect(uint32_t aHandle, const MFiWConnectPacket* aData);
        void SendDisconnect(uint32_t aHandle);
        void SendInputState(uint32_t aHandle, const MFiWInputStatePacket* aData);
        void SendStartDiscovery();
        void SendStopDiscovery();
        void SendSetPlayerIndex(uint32_t aHandle, int32_t aIndex);
        void SendPausePressed(uint32_t aHandle);

        virtual void HandlePacket(const MFiWDataPacket* aPacket) = 0;

    private:
        void SendGenericPacket(MFiWPacketType aType, uint32_t aHandle = 0);
        bool Read();
        void Parse();    
        static void Callback(CFSocketRef s, CFSocketCallBackType callbackType,
                             CFDataRef address, const void *data, void *info);
    
    
        int Descriptor;
        CFSocketRef Socket;
        CFRunLoopSourceRef Source;

        MFiWDataPacket Packet;
        uint8_t* Data;
        uint32_t Position;
};

}
