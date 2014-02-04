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
#include <math.h>
#include "HIDManager.h"
#include "HIDPad.h"
#include "protocol.h"

#include "sicksaxis.h"

HIDPad::Playstation3::Playstation3(HIDManager::Connection* aConnection)
    : Interface(aConnection), pauseHeld(true), needSetReport(true)
{
#ifdef IOS
    // Magic packet to start reports
    static uint8_t data[] = {0x53, 0xF4, 0x42, 0x03, 0x00, 0x00};
    HIDManager::SendPacket(aConnection, data, 6);
#endif

    FinalizeConnection();
}
 
void HIDPad::Playstation3::SetPlayerIndex(int32_t aIndex)
{
    if (aIndex < 0 || aIndex >= 4)
        ledByte = 0;
    else
        ledByte = 1 << (aIndex + 1);

    needSetReport = true;
}

void HIDPad::Playstation3::HandlePacket(uint8_t* aData, uint16_t aSize)
{
    if (needSetReport)
    {
        SetReport();
        needSetReport = false;    
    }

    SS_GAMEPAD* pad = (SS_GAMEPAD*)&aData[1];
        
    MFiWInputStatePacket data;
    memset(&data, 0, sizeof(data));
    
    #define B(X, Y) (((float)pad->X##_sens.Y) / 255.0f)
    
    data.A             = B(button, cross);
    data.B             = B(button, circle);
    data.X             = B(button, square);
    data.Y             = B(button, triangle);
    data.LeftShoulder  = B(shoulder, L1);
    data.RightShoulder = B(shoulder, R1);
    data.LeftTrigger   = B(shoulder, L2);
    data.RightTrigger  = B(shoulder, R2);

    data.DPadX = (B(dpad, left) > 0) ? -B(dpad, left) : B(dpad, right);
    data.DPadY = (B(dpad, up  ) > 0) ? -B(dpad, up)   : B(dpad, down );
    
    // Axes
    static const int32_t calibration[4] = { 0, 116, 140, 255 };
    data.LeftStickX = CalculateAxis(pad->left_analog.x, calibration);
    data.LeftStickY = CalculateAxis(pad->left_analog.y, calibration);
    data.RightStickX = CalculateAxis(pad->right_analog.x, calibration);
    data.RightStickY = CalculateAxis(pad->right_analog.y, calibration);

    if (!pauseHeld && pad->buttons.PS)
        MFiWrapperBackend::SendPausePressed(this);
    pauseHeld = pad->buttons.PS;

    MFiWrapperBackend::SendControllerState(this, &data);
}

const char* HIDPad::Playstation3::GetVendorName() const
{
    return "SixAxis/DualShock 3";
}

uint32_t HIDPad::Playstation3::GetPresentControls() const
{
    return MFi_AllElements;
}

uint32_t HIDPad::Playstation3::GetAnalogControls() const
{
    return MFi_AllElements;
}

void HIDPad::Playstation3::SetReport()
{
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
    report_buffer[11] = ledByte;
    //report_buffer[4] = motors[1] >> 8;
    //report_buffer[6] = motors[0] >> 8;
    HIDManager::SendPacket(connection, report_buffer, sizeof(report_buffer));
}