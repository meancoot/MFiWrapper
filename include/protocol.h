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
#include <stddef.h>

#define MFiWLocalHandle 0xFFFFFFFF

typedef enum
{
    MFi_DPad,               MFi_A,
    MFi_B,                  MFi_X,
    MFi_Y,                  MFi_LeftShoulder,
    MFi_RightShoulder,      MFi_LeftThumbstick,
    MFi_RightThumbstick,    MFi_LeftTrigger,
    MFi_RightTrigger,       MFi_Select,
    MFi_Start,              MFi_LeftStickButton,
    MFi_RightStickButton,   MFi_LastElement,
    MFi_NormalElements      = 0x7F,
    MFi_ExtendedElements    = 0x780,
    MFi_FullElements        = 0x7800,
    MFi_AllElements         = 0x7FFF,
}   MFiButtons;

typedef enum
{
#define DEFBIT(X) MFi_##X##_Bit = (1 << MFi_X)

    DEFBIT(DPad),               DEFBIT(A),
    DEFBIT(B),                  DEFBIT(X),
    DEFBIT(Y),                  DEFBIT(LeftShoulder),
    DEFBIT(RightShoulder),      DEFBIT(LeftThumbstick),
    DEFBIT(RightThumbstick),    DEFBIT(LeftTrigger),
    DEFBIT(RightTrigger),       DEFBIT(Select),
    DEFBIT(Start),              DEFBIT(LeftStickButton),
    DEFBIT(RightStickButton)
}   MFiButtonMask;

#pragma pack(push, 1)

typedef struct
{
    char VendorName[256];
    uint32_t PresentControls;
    uint32_t AnalogControls;
}   MFiWConnectPacket;

typedef struct
{
    // NOTE: This structure is laid out deliberately to mimic the
    //       expected layout of GCGamepadSnapshot's data.
    float DPadX;
    float DPadY;

    float A;
    float B;
    float X;
    float Y;
    float LeftShoulder;
    float RightShoulder;

    // Extended Gamepad
    float LeftStickX;
    float LeftStickY;
    float RightStickX;
    float RightStickY;

    float LeftTrigger;
    float RightTrigger;
    
    // Full Gamepad (Extension)
    float Select;
    float Start;
    float LeftStickButton;
    float RightStickButton;

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
    MFiWPacketSetPlayerIndex,
    MFiWPacketPausePressed,
    MFiWPacketLast,
    MFiWPacketUINT32 = 0xFFFFFFFF
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

static_assert(offsetof(MFiWDataPacket, Connect) == 12,
              "MFiWDataPacket header must be 12 bytes long.");

#define MFiWPacketGenericSize        (12                                )
#define MFiWPacketConnectSize        (12 + sizeof(MFiWConnectPacket)    )
#define MFiWPacketInputStateSize     (12 + sizeof(MFiWInputStatePacket) )
#define MFiWPacketSetPlayerIndexSize (12 + sizeof(MFiWPlayerIndexPacket))

#pragma pack(pop)
