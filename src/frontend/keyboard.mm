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
#include "frontend.h"
#include "keyboard.h"

// This code is run in the MFiWrapper frontend.
// It calls functions to simulate reports from the backend,
// but uses MFiWLocalHandle (0xFFFFFFFF) as the connection id.

// Give symbolic names to iCade buttons.
// Based on the buttons of the 8-bitty controller.
enum iCade8BittySym
{
    UP,    DOWN , LEFT,   RIGHT,
    UP_LF, UP_RT, LW_LF,  LW_RT,
    L1,    R1,    SELECT, START
};

// Map Keyboard HID usage codes to iCade8BittySym values.
// iCadeMap[4] is equivalent to the 'A' key, iCadeMap[5] is 'B', and so on.
static const struct
{
  bool Up;
  int32_t Button;
}  iCadeMap[0x20] =
{
  { false, -1    }, { false, -1    }, { false, -1   }, { false, -1    }, // 0
  { false, LEFT  }, { false, -1    }, { true , RIGHT}, { false, RIGHT }, // 4
  { true , UP    }, { true,  START }, { true , UP_RT}, { false, L1    }, // 8
  { false, UP_LF }, { false, R1    }, { false, LW_LF}, { false, LW_RT }, // C
  { true , UP_LF }, { true , R1    }, { false, UP_RT}, { true,  LW_LF }, // 0
  { true , LEFT  }, { true , L1    }, { false, -1   }, { true , SELECT}, // 4
  { false, START }, { true , LW_RT }, { false, UP   }, { false, DOWN  }, // 8
  { false, SELECT}, { true , DOWN  }, { false, -1  },  { false, -1    }  // C
};

// This holds the current state of the iCade inputs.
static MFiWDataPacket dataPacket;

void MFiWrapperFrontend::Keyboard::Event(bool aDown, uint32_t aCode)
{
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;

        MFiWDataPacket pkt;
        memset(&pkt, 0, sizeof(pkt));
        pkt.Size = 12 + sizeof(pkt.Connect);
        pkt.Type = MFiWPacketConnect;
        pkt.Handle = MFiWLocalHandle;
        strlcpy(pkt.Connect.VendorName, "iCade Device", sizeof(pkt.Connect.VendorName));
        pkt.Connect.PresentControls = MFi_NormalElements;
        HandlePacketConnect(&pkt);
        
        //
        dataPacket.Size = 12 + sizeof(pkt.State);
        dataPacket.Type = MFiWPacketInputState;
        dataPacket.Handle = MFiWLocalHandle;
    }

    if (aDown && aCode < 0x20 && iCadeMap[aCode].Button >= 0)
    {
        switch (iCadeMap[aCode].Button)
        {
            #define CHECK(X) ((iCadeMap[aCode].Up) ? 0.0f : (X))
            case LW_LF: dataPacket.State.A = CHECK(1.0f); break;
            case LW_RT: dataPacket.State.B = CHECK(1.0f); break;
            case UP_LF: dataPacket.State.X = CHECK(1.0f); break;
            case UP_RT: dataPacket.State.Y = CHECK(1.0f); break;
            case L1:    dataPacket.State.LeftShoulder = CHECK(1.0f); break;
            case R1:    dataPacket.State.RightShoulder = CHECK(1.0f); break;
            
            case UP:    dataPacket.State.DPadY = CHECK(-1.0f); break;
            case DOWN:  dataPacket.State.DPadY = CHECK(1.0f); break;
            case LEFT:  dataPacket.State.DPadX = CHECK(-1.0f); break;
            case RIGHT: dataPacket.State.DPadX = CHECK(1.0f); break;
            
            case START:
            {
                MFiWDataPacket pkt;
                memset(&pkt, 0, sizeof(pkt));
                pkt.Size = 12;
                pkt.Type = MFiWPacketPausePressed;
                pkt.Handle = MFiWLocalHandle;
                HandlePacketPausePressed(&pkt);
                return;
            }    
        }
        
        HandlePacketInputState(&dataPacket);
    }
}
