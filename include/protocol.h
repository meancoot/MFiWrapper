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

typedef enum MFiButtons
{
    MFi_A, MFi_B, MFi_X, MFi_Y, MFi_LeftShoulder, MFi_RightShoulder,
    MFi_LeftTrigger, MFi_RightTrigger, MFi_Up, MFi_Down, MFi_Left, MFi_Right,
    MFi_LeftUp, MFi_LeftDown, MFi_LeftLeft, MFi_LeftRight,
    MFi_RightUp, MFi_RightDown, MFi_RightLeft, MFi_RightRight,
    MFi_LastButton
}   MFiButtons;

typedef struct ConnectionOpenPacket
{
    char VendorName[256];
    uint32_t PresentControls;
    uint32_t AnalogControls;
}   ConnectionOpenPacket;

typedef struct ConnectionClosedPacket
{
    int dummy;
}   ConnectionClosedPacket;

typedef struct StatePacket
{
    float Data[32];
}   StatePacket;

typedef enum PacketType { PKT_OPEN, PKT_CLOSE, PKT_STATE } PacketType;

typedef struct DataPacket
{
    uint32_t Size;
    PacketType Type;
    uint32_t Handle;
    
    union
    {
        ConnectionOpenPacket Open;
        ConnectionClosedPacket Closed;
        StatePacket State;
    };
}   DataPacket;