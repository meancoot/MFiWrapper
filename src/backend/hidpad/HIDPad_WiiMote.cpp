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
    static const uint32_t buttonMap[][2] =
    {
        { MFi_A, WIIMOTE_BUTTON_ONE },
        { MFi_B, WIIMOTE_BUTTON_TWO },
        { MFi_X, WIIMOTE_BUTTON_A },
        { MFi_Y, WIIMOTE_BUTTON_B },
        { MFi_LeftShoulder, WIIMOTE_BUTTON_MINUS },
        { MFi_RightShoulder, WIIMOTE_BUTTON_PLUS },
        { MFi_Up, WIIMOTE_BUTTON_UP },
        { MFi_Down, WIIMOTE_BUTTON_DOWN },
        { MFi_Left, WIIMOTE_BUTTON_LEFT },
        { MFi_Right, WIIMOTE_BUTTON_RIGHT },
        { 0xFFFFFFFF, 0xFFFFFFFF }
    };
    
    float data[32];
    memset(data, 0, sizeof(data));
    
    for (int i = 0; buttonMap[i][0] != 0xFFFFFFFF; i ++)
        data[buttonMap[i][0]] = (device.btns & buttonMap[i][1]) ? 1.0f : 0.0f;
    MFiWrapperBackend::SendControllerState(this, data);
}

const char* HIDPad::WiiMote::GetVendorName() const
{
    return "Wii Remote";
}
