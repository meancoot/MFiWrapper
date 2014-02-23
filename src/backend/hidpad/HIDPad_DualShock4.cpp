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

HIDPad::DualShock4::DualShock4(HIDManager::Connection* aConnection)
    : Interface(aConnection), pauseHeld(true), playerIndex(-1), needSetReport(true)
{
    // This is needed to get full input packet over bluetooth.
    uint8_t buf[0x25];
    HIDManager::GetReport(connection, true, 0x2, buf, sizeof(buf));

    FinalizeConnection();
}

void HIDPad::DualShock4::SetPlayerIndex(int32_t aIndex)
{
    playerIndex = aIndex;
    needSetReport = true;
}

void HIDPad::DualShock4::HandlePacket(uint8_t* aData, uint16_t aSize)
{
    if (needSetReport)
        SetReport();
    needSetReport = false;

    // Only handle valid input packets
    if (aSize != 79 || aData[2] != 0xC0 || aData[3])
    {
        return;
    }

    struct Report
    {
        uint8_t leftX;
        uint8_t leftY;
        uint8_t rightX;
        uint8_t rightY;
        uint8_t buttons[3];
        uint8_t leftTrigger;
        uint8_t rightTrigger;
    };

    Report* rpt = (Report*)&aData[4];

    MFiWInputStatePacket data;
    memset(&data, 0, sizeof(data));

    #define B(X)            (X) ? 1.0f : 0.0f;
    data.A                  = B(rpt->buttons[0] & 0x20); // Cross
    data.B                  = B(rpt->buttons[0] & 0x40); // Circle
    data.X                  = B(rpt->buttons[0] & 0x10); // Square
    data.Y                  = B(rpt->buttons[0] & 0x80); // Triangle
    data.LeftShoulder       = B(rpt->buttons[1] & 0x01); // L1
    data.RightShoulder      = B(rpt->buttons[1] & 0x02); // R1
    data.LeftTrigger        = B(rpt->buttons[1] & 0x04); // L2 (TODO: Use analog)
    data.RightTrigger       = B(rpt->buttons[1] & 0x08); // R2 (TODO: Use analog)
    data.Select             = B(rpt->buttons[1] & 0x10); // Share
    data.Start              = B(rpt->buttons[1] & 0x20); // Options
    data.LeftStickButton    = B(rpt->buttons[1] & 0x40); // L3
    data.RightStickButton   = B(rpt->buttons[1] & 0x80); // R3

    // DPad
    static const float dpadStates[8][2] =
    {
        {  0, -1 }, {  1, -1 }, {  1,  0 }, {  1,  1 },
        {  0,  1 }, { -1,  1 }, { -1,  0 }, { -1, -1 }
    };
    const uint8_t dpadState = rpt->buttons[0] & 0xF;
    if (dpadState < 8)
    {
        data.DPadX = dpadStates[dpadState][0];
        data.DPadY = dpadStates[dpadState][1];
    }

    // Axes
    static const int32_t calibration[4] = { 0, 100, 155, 255 };
    data.LeftStickX = CalculateAxis(rpt->leftX, calibration);
    data.LeftStickY = 0.0f - CalculateAxis(rpt->leftY, calibration);
    data.RightStickX = CalculateAxis(rpt->rightX, calibration);
    data.RightStickY = 0.0f - CalculateAxis(rpt->rightY, calibration);

    if (!pauseHeld && rpt->buttons[2] & 0x01) // PS Button
        MFiWrapperBackend::SendPausePressed(this);
    pauseHeld = rpt->buttons[2] & 0x01;

    MFiWrapperBackend::SendControllerState(this, &data);
}

const char* HIDPad::DualShock4::GetVendorName() const
{
    return "DualShock 4";
}

uint32_t HIDPad::DualShock4::GetPresentControls() const
{
    return MFi_AllElements;
}

uint32_t HIDPad::DualShock4::GetAnalogControls() const
{
    return MFi_LeftThumbstick_Bit | MFi_RightThumbstick_Bit;
}

void HIDPad::DualShock4::SetReport()
{
    static uint8_t report_buffer[79] = {
        0x52, 0x11, 0xB0, 0x00, 0x0F
    };

    uint8_t rgb[4][3] { { 0xFF, 0, 0 }, { 0, 0xFF, 0 }, { 0, 0, 0xFF }, { 0xFF, 0xFF, 0xFF } };

    if (playerIndex >= 0 && playerIndex < 4)
    {
        report_buffer[ 9] = rgb[playerIndex][0];
        report_buffer[10] = rgb[playerIndex][1];
        report_buffer[11] = rgb[playerIndex][2];
    }

    HIDManager::SetReport(connection, false, 0x11, report_buffer, sizeof(report_buffer));
}
