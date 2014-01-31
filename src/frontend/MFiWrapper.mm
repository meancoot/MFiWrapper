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

#include "MFiWrapper.h"

struct UpdatePacket
{
    UpdatePacket(GCController* aController) : 
        controller(aController), buttonCount(0)
    {
        memset(buttonIndex, 0, sizeof(buttonIndex));
        memset(buttonData, 0, sizeof(buttonData));
    }

    void AddButton(unsigned aIndex, float aValue)
    {
        buttonIndex[buttonCount] = aIndex;
        buttonData[buttonCount] = aValue;
        buttonCount ++;
    }

    void Send()
    {
        if (!buttonCount)
        {
            delete this;
            return;
        }
    
        dispatch_async(dispatch_get_main_queue(), ^{             
            for (unsigned i = 0; i < buttonCount; i ++)
            {
                GCControllerButtonInput* button = controller.tweakButtons[buttonIndex[i]];
                button.pressed = buttonData[i] > .25f;
                button.value = buttonData[i];
                        
                if (button.valueChangedHandler)
                    button.valueChangedHandler(button, button.value, button.pressed);                
            }
            
            delete this;
        });
    }

    GCController* controller;
    unsigned buttonCount;
    unsigned buttonIndex[MFi_LastButton];
    float buttonData[MFi_LastButton];
};

class GCHIDPadListener : public HIDPad::Listener
{
    public:
        GCHIDPadListener(GCController* aController) : controller(aController)
        {
            memset(buttonData, 0, sizeof(buttonData));
        }
        
        virtual void SetButtons(uint32_t aFirst, uint32_t aCount, float* aData)
        {        
            // TODO: Don't allocate and delete every time, use a cache.
            UpdatePacket* packet = new UpdatePacket(controller);
        
            for (uint32_t idx = 0, btn = aFirst; idx < aCount && btn < MFi_LastButton; idx ++, btn ++)
            {
                if (buttonData[idx] != aData[btn])
                {
                    packet->AddButton(idx, aData[idx]);                
                    buttonData[idx] = aData[btn];
                }
            }
            
            packet->Send();
        }

        virtual void SetAxes(uint32_t aFirst, uint32_t aCount, float* aData)
        {
            // TODO        
        }
        
    protected:
        GCController* controller;
        float buttonData[MFi_LastButton];
};

/****************/
/* GCController */
/****************/
@implementation GCController

- (void)dealloc
{
    [_gamepad release];
    [_extendedGamepad release];
    [_controllerPausedHandler release];
    [_tweakButtons release];
    [_tweakAxis release];

    [super dealloc];
}

- (NSString*)vendorName
{
    return [NSString stringWithUTF8String:self.tweakHIDPad->GetVendorName()];
}

+ (GCController*)controllerForHIDPad:(HIDPad::Interface*)hidpad
{
    GCController* tweak = [GCController new];

    tweak.tweakButtons = [NSMutableArray array];
    for (int i = 0; i != 20; i ++)
        [tweak.tweakButtons addObject:[GCControllerButtonInput buttonForController:tweak index:i]];
        
    tweak.tweakAxis = [NSMutableArray array];
    for (int i = 0; i != 6; i ++)
        [tweak.tweakAxis addObject:[GCControllerAxisInput axisForController:tweak index:i]];

    tweak.tweakHIDPad = hidpad;
    tweak.gamepad = [GCGamepad gamepadForController:tweak];
    tweak.extendedGamepad = [GCExtendedGamepad gamepadForController:tweak];
    tweak.playerIndex = GCControllerPlayerIndexUnset;

    hidpad->SetListener(new GCHIDPadListener(tweak));

    return tweak;
}

- (void)setPlayerIndex:(NSInteger)index
{
    self.tweakHIDPad->SetPlayerIndex(index);
}

- (NSInteger)playerIndex
{
    return self.tweakHIDPad->GetPlayerIndex();
}

@end

/*************/
/* GCGamepad */
/*************/
@implementation GCGamepad

- (void)dealloc
{
    [_dpad release];    
    [super dealloc];
}

+ (GCGamepad*)gamepadForController:(GCController*)controller
{
    GCGamepad* tweak = [GCGamepad new];
    
    tweak.controller = controller;
    tweak.buttonA = controller.tweakButtons[0];
    tweak.buttonB = controller.tweakButtons[1];
    tweak.buttonX = controller.tweakButtons[2];
    tweak.buttonY = controller.tweakButtons[3];
    
    tweak.leftShoulder = controller.tweakButtons[4];
    tweak.rightShoulder = controller.tweakButtons[5];

    tweak.dpad = [GCControllerDirectionPad dpadForController:controller index:0];
    
    return tweak;
}

- (GCGamepadSnapshot*)saveSnapshot
{
    return nil;
}

@end

/*********************/
/* GCExtendedGamepad */
/*********************/
@implementation GCExtendedGamepad

- (void)dealloc
{
    [_dpad release];
    [_leftThumbstick release];
    [_rightThumbstick release];
    
    [super dealloc];
}

+ (GCExtendedGamepad*)gamepadForController:(GCController*)controller
{
    GCExtendedGamepad* tweak = [GCExtendedGamepad new];
    
    tweak.controller = controller;
    tweak.buttonA = controller.tweakButtons[0];
    tweak.buttonB = controller.tweakButtons[1];
    tweak.buttonX = controller.tweakButtons[2];
    tweak.buttonY = controller.tweakButtons[3];
    
    tweak.leftShoulder = controller.tweakButtons[4];
    tweak.rightShoulder = controller.tweakButtons[5];
    tweak.leftTrigger = controller.tweakButtons[6];
    tweak.rightTrigger = controller.tweakButtons[7];

    tweak.dpad = [GCControllerDirectionPad dpadForController:controller index:0];
    tweak.leftThumbstick = [GCControllerDirectionPad dpadForController:controller index:1];
    tweak.rightThumbstick = [GCControllerDirectionPad dpadForController:controller index:2];
    
    return tweak;
}

- (GCExtendedGamepadSnapshot*)saveSnapshot
{
    return nil;
}

@end

/***********************/
/* GCControllerElement */
/***********************/
@implementation GCControllerElement
@end

/***************************/
/* GCControllerButtonInput */
/***************************/
@implementation GCControllerButtonInput : GCControllerElement

+ (GCControllerButtonInput*)buttonForController:(GCController*)controller index:(NSInteger)index
{
    GCControllerButtonInput* tweak = [GCControllerButtonInput new];
    tweak.tweakController = controller;
    tweak.tweakIndex = index;
    return tweak;
}

@end

/*************************/
/* GCControllerAxisInput */
/*************************/
@implementation GCControllerAxisInput

+ (GCControllerAxisInput*)axisForController:(GCController*)controller index:(NSInteger)index
{
    GCControllerAxisInput* tweak = [GCControllerAxisInput new];
    tweak.tweakController = controller;
    tweak.tweakIndex = index;
    return tweak;
}

@end

/****************************/
/* GCControllerDirectionPad */
/****************************/
@implementation GCControllerDirectionPad : GCControllerElement

+ (GCControllerDirectionPad*)dpadForController:(GCController*)controller index:(NSInteger)index
{
    GCControllerDirectionPad* tweak = [GCControllerDirectionPad new];
    tweak.tweakController = controller;
    tweak.tweakIndex = index;

    tweak.xAxis = controller.tweakAxis[index * 2 + 0];
    tweak.xAxis.collection = tweak;
    
    tweak.yAxis = controller.tweakAxis[index * 2 + 0];
    tweak.yAxis.collection = tweak;

    tweak.up = controller.tweakButtons[8 + (index * 4) + 0];
    tweak.up.collection = tweak;
    
    tweak.down = controller.tweakButtons[8 + (index * 4) + 1];
    tweak.down.collection = tweak;
    
    tweak.left = controller.tweakButtons[8 + (index * 4) + 2];
    tweak.left.collection = tweak;
    
    tweak.right = controller.tweakButtons[8 + (index * 4) + 3];
    tweak.right.collection = tweak;

    return tweak;
}

@end
