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

#include <stdint.h>

typedef enum
{
    MFi_A,              MFi_B,
    MFi_X,              MFi_Y,
    MFi_LeftShoulder,   MFi_RightShoulder,
    MFi_LeftTrigger,    MFi_RightTrigger,
    MFi_LastButton,     MFi_DPad = MFi_LastButton,
    MFi_LeftThumbstick, MFi_RightThumbstick,
    MFi_LastDPad,       MFi_LastInput = MFi_LastDPad
}   MFiButtons;

#pragma pack(push, 1)

typedef struct
{
    char VendorName[256];
    uint32_t PresentControls;
    uint32_t AnalogControls;
}   MFiWConnectPacket;

typedef union
{
    struct
    {
        float A;
        float B;
        float X;
        float Y;
        float LeftShoulder;
        float RightShoulder;
        float LeftTrigger;
        float RightTrigger;
        float DPadX;
        float DPadY;
        float LeftStickX;
        float LeftStickY;
        float RightStickX;
        float RightSitckY;
    };

    float Data[32];
}   MFiWInputStatePacket;

typedef struct
{
    int32_t Value;
}   MFiWPlayerIndexPacket;

typedef enum 
{
    MFiWPacketConnect,
    MFiWPacketDisconnect,
    MFiWPacketInputState,
    MFiWPacketStartDiscovery,
    MFiWPacketStopDiscovery,
    MFiWPacketSetPlayerIndex,
    MFiWPacketLast = 0xFFFFFFFF
}   MFiWPacketType;

typedef struct
{
    uint32_t Size;
    MFiWPacketType Type;
    uint32_t Handle;
    
    union
    {
        MFiWConnectPacket Connect;
        MFiWInputStatePacket State;
        MFiWPlayerIndexPacket PlayerIndex;
    };
}   MFiWDataPacket;

#pragma pack(pop)
