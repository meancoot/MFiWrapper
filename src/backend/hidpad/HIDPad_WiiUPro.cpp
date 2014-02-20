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
#include <algorithm>

#include "HIDManager.h"
#include "HIDPad.h"
#include "protocol.h"

HIDPad::WiiUPro::WiiUPro(HIDManager::Connection* aConnection)
    : Interface(aConnection), pauseHeld(true), playerIndex(-1), needSetReport(true),
      calibrationInitialized(false)
{
    FinalizeConnection();

    // 0x15 = Request Status
    uint8_t data[4] = { 0xA2, 0x15, 0x00 };
    HIDManager::SetReport(connection, false, 0x12, data, sizeof(data));
}
 
void HIDPad::WiiUPro::SetPlayerIndex(int32_t aIndex)
{
    playerIndex = aIndex;
    needSetReport = true;
}

void HIDPad::WiiUPro::HandlePacket(uint8_t* aData, uint16_t aSize)
{
    if (needSetReport)
        SetReport();
    needSetReport = false;

    if (aSize != 23 || aData[1] != 0x34)
    {
        return;
    }

    aData[0x0C] ^= 0xFF;
    aData[0x0D] ^= 0xFF;
    aData[0x0E] ^= 0xFF;
        
    MFiWInputStatePacket data;
    memset(&data, 0, sizeof(data));
    
    #define B(X)            ((X) ? 1.0f : 0.0f)
    data.A                  = B(aData[0x0D] & 0x40); // B
    data.B                  = B(aData[0x0D] & 0x10); // A
    data.X                  = B(aData[0x0D] & 0x20); // Y
    data.Y                  = B(aData[0x0D] & 0x08); // X
    data.LeftShoulder       = B(aData[0x0C] & 0x20); // L
    data.RightShoulder      = B(aData[0x0C] & 0x02); // R
    data.LeftTrigger        = B(aData[0x0D] & 0x80); // ZL
    data.RightTrigger       = B(aData[0x0D] & 0x04); // ZR
    data.Select             = B(aData[0x0C] & 0x10); // Minus
    data.Start              = B(aData[0x0C] & 0x04); // Plus
    data.LeftStickButton    = B(aData[0x0E] & 0x02); // Left Stick
    data.RightStickButton   = B(aData[0x0E] & 0x01); // Right Stick    

    data.DPadX = (aData[0x0D] & 0x02) ? -B(aData[0x0D] & 0x02) : B(aData[0x0C] & 0x80); // Left-Right
    data.DPadY = (aData[0x0D] & 0x01) ? -B(aData[0x0D] & 0x01) : B(aData[0x0C] & 0x40); // Up-Down
    
    // Axes
    #define FETCH_16(x) (aData[x] | (aData[x + 1] << 8))
    struct { float* data; uint32_t offset; int32_t* calibration; } axis[4] =
    {
        { &data.LeftStickX,   4, calibration[0] },
        { &data.LeftStickY,   8, calibration[1] },
        { &data.RightStickX,  6, calibration[2] },
        { &data.RightStickY, 10, calibration[3] }
    };
    
    for (int i = 0; i != 4; i ++)
    {
        int32_t value = FETCH_16(axis[i].offset);
        
        if (!calibrationInitialized)
        {
            axis[i].calibration[0] = value - 0x100;
            axis[i].calibration[1] = value - 0x80;
            axis[i].calibration[2] = value + 0x80;
            axis[i].calibration[3] = value + 0x100;
        }
        
        axis[i].calibration[0] = std::min(value, axis[i].calibration[0]);
        axis[i].calibration[3] = std::max(value, axis[i].calibration[3]);
        
        *axis[i].data = CalculateAxis(value, axis[i].calibration);
    }
    
    calibrationInitialized = true;

    if (!pauseHeld && aData[0x0C] & 0x8) // Home Button
        MFiWrapperBackend::SendPausePressed(this);
    pauseHeld = aData[0x0C] & 0x8;

    MFiWrapperBackend::SendControllerState(this, &data);
}

const char* HIDPad::WiiUPro::GetVendorName() const
{
    return "Wii U Pro Controller";
}

uint32_t HIDPad::WiiUPro::GetPresentControls() const
{
    return MFi_AllElements;
}

uint32_t HIDPad::WiiUPro::GetAnalogControls() const
{
    return MFi_LeftThumbstick_Bit | MFi_RightThumbstick_Bit;
}

void HIDPad::WiiUPro::SetReport()
{
    // 0x12 = Set data report; 0x34 = All Buttons and Analogs
    static uint8_t data[4] = { 0xA2, 0x12, 0x00, 0x34 };
    HIDManager::SetReport(connection, false, 0x12, data, sizeof(data));

    if (playerIndex >= 0 && playerIndex < 4)
    {
        // 0x11 = Set player leds
        uint8_t leds[3] = { 0xA2, 0x11, 0x00 };
        leds[2] = 1 << (4 + playerIndex);
        HIDManager::SetReport(connection, false, 0x11, leds, sizeof(leds));
    }
}
