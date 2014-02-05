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

#include <string.h>
#include "HIDManager.h"
#include "HIDPad.h"
#include "protocol.h"

HIDPad::WiiMote::WiiMote(HIDManager::Connection* aConnection) : Interface(aConnection)
{            
    memset(&device, 0, sizeof(device));

    device.connection = connection;
    device.unid = -1;
    device.state = WIIMOTE_STATE_CONNECTED;
    device.exp.type = EXP_NONE;

    wiimote_handshake(&device, -1, NULL, -1);
    
    FinalizeConnection();
}            

void HIDPad::WiiMote::SetPlayerIndex(int32_t aIndex)
{
    Interface::SetPlayerIndex(aIndex);            
    device.unid = aIndex;
    
    switch (aIndex)
    {
        case 0: wiimote_set_leds(&device, WIIMOTE_LED_1); break;
        case 1: wiimote_set_leds(&device, WIIMOTE_LED_2); break;
        case 2: wiimote_set_leds(&device, WIIMOTE_LED_3); break;
        case 3: wiimote_set_leds(&device, WIIMOTE_LED_4); break;                                                     
    }
}

void HIDPad::WiiMote::HandlePacket(uint8_t *aData, uint16_t aSize)
{
    byte* msg = aData + 2;
    switch (aData[1])
    {
        case WM_RPT_BTN:
        {
            wiimote_pressed_buttons(&device, msg);
            break;
        }

        case WM_RPT_READ:
        {
            wiimote_pressed_buttons(&device, msg);
            wiimote_handshake(&device, WM_RPT_READ, msg + 5, ((msg[2] & 0xF0) >> 4) + 1);
            break;
        }

        case WM_RPT_CTRL_STATUS:
        {
            wiimote_pressed_buttons(&device, msg);
            wiimote_handshake(&device,WM_RPT_CTRL_STATUS,msg,-1);
            break;
        }

        case WM_RPT_BTN_EXP:
        {
            wiimote_pressed_buttons(&device, msg);
            wiimote_handle_expansion(&device, msg+2);
            break;
        }
    }
    
    ProcessButtons();
}

void HIDPad::WiiMote::ProcessButtons()
{
    MFiWInputStatePacket data;
    memset(&data, 0, sizeof(data));
    
    if (device.exp.type == EXP_NONE)
    {   
        #define B(X) (device.btns & WIIMOTE_BUTTON_##X)
    
        data.A              = B(ONE)   ? 1.0f : 0.0f;
        data.B              = B(TWO)   ? 1.0f : 0.0f;
        data.X              = B(A)     ? 1.0f : 0.0f;
        data.Y              = B(B)     ? 1.0f : 0.0f;
        data.LeftShoulder   = B(MINUS) ? 1.0f : 0.0f;
        data.RightShoulder  = B(PLUS)  ? 1.0f : 0.0f;
    
             if (B(UP))        data.DPadX = -1.0f;
        else if (B(DOWN))      data.DPadX =  1.0f;
    
             if (B(RIGHT))     data.DPadY = -1.0f;
        else if (B(LEFT))      data.DPadY =  1.0f;
        
        #undef B
    }
    else if (device.exp.type == EXP_CLASSIC)
    {
        #define B(X) (device.exp.classic.btns & CLASSIC_CTRL_BUTTON_##X)
        data.A              = B(B)     ? 1.0f : 0.0f;
        data.B              = B(A)     ? 1.0f : 0.0f;
        data.X              = B(Y)     ? 1.0f : 0.0f;
        data.Y              = B(X)     ? 1.0f : 0.0f;
        data.LeftShoulder   = B(FULL_L)? 1.0f : 0.0f;
        data.RightShoulder  = B(FULL_R)? 1.0f : 0.0f;

        data.LeftTrigger    = B(ZL)    ? 1.0f : 0.0f;
        data.RightTrigger   = B(ZR)    ? 1.0f : 0.0f;
        
             if (B(LEFT))      data.DPadX = -1.0f;
        else if (B(RIGHT))     data.DPadX =  1.0f;
    
             if (B(UP))        data.DPadY = -1.0f;
        else if (B(DOWN))      data.DPadY =  1.0f;
        
        data.LeftStickX     = device.exp.classic.ljs.x.value;
        data.LeftStickY     = device.exp.classic.ljs.y.value;
        data.RightStickX    = device.exp.classic.rjs.x.value;
        data.RightStickY    = device.exp.classic.rjs.y.value;                        
    }

    MFiWrapperBackend::SendControllerState(this, &data);
}

const char* HIDPad::WiiMote::GetVendorName() const
{
    return "Wii Remote";
}
