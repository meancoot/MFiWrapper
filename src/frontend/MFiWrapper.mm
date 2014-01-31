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
#include "protocol.h"

/****************/
/* GCController */
/****************/
@implementation GCController

+ (GCController*)controllerForHandle:(uint32_t)handle data:(struct ConnectionOpenPacket)data
{
    GCController* tweak = [GCController new];
    tweak.tweakHandle = handle;
    tweak.vendorName = [NSString stringWithUTF8String:data.VendorName];

    tweak.tweakButtons = [NSMutableArray array];
    for (int i = 0; i != 20; i ++)
    {
        GCControllerButtonInput* button = [GCControllerButtonInput buttonForController:tweak index:i];
        button.analog = (data.AnalogControls & (1 << i)) ? YES : NO;
        [tweak.tweakButtons addObject:button];
    }
        
    tweak.tweakAxis = [NSMutableArray array];
    for (int i = 0; i != 6; i ++)
    {
        GCControllerAxisInput* axis = [GCControllerAxisInput axisForController:tweak index:i];
        axis.analog = (data.AnalogControls & (1 << (20 + 1))) ? YES : NO;
        [tweak.tweakAxis addObject:axis];
    }

    tweak.gamepad = [GCGamepad gamepadForController:tweak];
    tweak.extendedGamepad = [GCExtendedGamepad gamepadForController:tweak];
    tweak.playerIndex = GCControllerPlayerIndexUnset;

    return [tweak autorelease];
}

- (void)dealloc
{
    [_controllerPausedHandler release];
    [_vendorName release];
    [_gamepad release];
    [_extendedGamepad release];
    [_controllerPausedHandler release];
    [_tweakButtons release];
    [_tweakAxis release];

    [super dealloc];
}

// TODO: PLAYER INDEX

@end

/*************/
/* GCGamepad */
/*************/
@implementation GCGamepad

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
    
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [_dpad release];    
    [super dealloc];
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
    
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [_dpad release];
    [_leftThumbstick release];
    [_rightThumbstick release];
    
    [super dealloc];
}

- (GCExtendedGamepadSnapshot*)saveSnapshot
{
    return nil;
}

@end

/***********************/
/* GCControllerElement */
/***********************/
@implementation GCControllerElement @end

/***************************/
/* GCControllerButtonInput */
/***************************/
@implementation GCControllerButtonInput : GCControllerElement

+ (GCControllerButtonInput*)buttonForController:(GCController*)controller index:(NSInteger)index
{
    GCControllerButtonInput* tweak = [GCControllerButtonInput new];
    tweak.tweakController = controller;
    tweak.tweakIndex = index;
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [super dealloc];
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
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [super dealloc];
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

    return [tweak autorelease];
}

- (void)delloc
{
    [_valueChangedHandler release];
    [super dealloc];
}

@end
