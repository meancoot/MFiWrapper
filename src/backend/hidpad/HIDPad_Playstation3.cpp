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

#include <stdio.h>
#include <string.h>
#include "HIDManager.h"
#include "HIDPad.h"
#include "protocol.h"

namespace PS3Bits
{
    enum
    {
        SELECT, LR, R3, START,
        UP, RIGHT, DOWN, LEFT,
        L2, R2, L1, R1,
        TRIANGLE, CIRCLE, CROSS, SQUARE,
        PS
    };
    
    inline bool Button(int buttons, int button) { return buttons & (1 << button); }
}

HIDPad::Playstation3::Playstation3(HIDManager::Connection* aConnection) : Interface(aConnection)
{
#ifdef IOS
    // Magic packet to start reports
    static uint8_t data[] = {0x53, 0xF4, 0x42, 0x03, 0x00, 0x00};
    HIDManager::SendPacket(device->connection, data, 6);
#endif

   // Without this the digital buttons won't be reported
   SetPlayerIndex(-1);
   
   FinalizeConnection();
}
 
void HIDPad::Playstation3::SetPlayerIndex(int32_t aIndex)
{
    Interface::SetPlayerIndex(aIndex);

    // TODO: Can this be modified to turn off motion tracking?
    static uint8_t report_buffer[] = {
        0x52, 0x01,
        0x00, 0xFF, 0x00, 0xFF, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00
    };

    report_buffer[11] = 1 << ((playerIndex % 4) + 1);
    //report_buffer[4] = motors[1] >> 8;
    //report_buffer[6] = motors[0] >> 8;
    HIDManager::SendPacket(connection, report_buffer, sizeof(report_buffer));
}

void HIDPad::Playstation3::HandlePacket(uint8_t *aData, uint16_t aSize)
{    
    uint32_t buttons = aData[3] | (aData[4] << 8) | ((aData[5] & 1) << 16);
    
    MFiWInputStatePacket data;
    memset(&data, 0, sizeof(data));
    
    using namespace PS3Bits;
    data.A             = Button(buttons, CROSS)    ? 1.0f : 0.0f;
    data.B             = Button(buttons, CIRCLE)   ? 1.0f : 0.0f;
    data.X             = Button(buttons, SQUARE)   ? 1.0f : 0.0f;
    data.Y             = Button(buttons, TRIANGLE) ? 1.0f : 0.0f;
    data.LeftShoulder  = Button(buttons, L1)       ? 1.0f : 0.0f;
    data.RightShoulder = Button(buttons, R1)       ? 1.0f : 0.0f;
    data.LeftTrigger   = Button(buttons, L2)       ? 1.0f : 0.0f;
    data.RightTrigger  = Button(buttons, R2)       ? 1.0f : 0.0f;
    
    if (Button(buttons, UP))
        data.DPadX = 1.0f;
    else if (Button(buttons, DOWN))
        data.DPadX = -1.0f;

    if (Button(buttons, LEFT))
        data.DPadY = -1.0f;
    else if (Button(buttons, RIGHT))
        data.DPadY = 1.0f;

    MFiWrapperBackend::SendControllerState(this, data.Data);
}

const char* HIDPad::Playstation3::GetVendorName() const
{
    return "SixAxis/DualShock 3";
}
