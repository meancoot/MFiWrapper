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

#include "frontend.h"
#include "MFiWrapper.h"
#include "protocol.h"

/****************/
/* GCController */
/****************/
@implementation GCController

+ (GCController*)controllerForHandle:(uint32_t)handle data:(MFiWConnectPacket)data
{
    GCController* tweak = [GCController new];
    tweak.tweakHandle = handle;
    tweak.vendorName = [NSString stringWithUTF8String:data.VendorName];

    #define ANALOG_TEST(X) analog:((data.AnalogControls & MFi_##X##_Bit) ? YES : NO)
    #define DPAD(X)   [GCControllerDirectionPad dpadWithParent:nil ANALOG_TEST(X)]
    #define BUTTON(X) [GCControllerButtonInput buttonWithParent:nil ANALOG_TEST(X)]

    tweak.tweakElements = [NSArray arrayWithObjects:
        // GCGamepad
        DPAD(DPad), BUTTON(A), BUTTON(B), BUTTON(X), BUTTON(Y),
        BUTTON(LeftShoulder), BUTTON(RightShoulder),
        DPAD(LeftThumbstick), DPAD(RightThumbstick),
        BUTTON(LeftTrigger), BUTTON(RightTrigger),
        nil
    ];
    
    #undef DPAD
    #undef BUTTON
    #undef ANALOG_TEST

    tweak.playerIndex = GCControllerPlayerIndexUnset;
    tweak.gamepad = [GCGamepad gamepadForController:tweak];
    
    if (data.PresentControls & MFi_ExtendedElements)
        tweak.extendedGamepad = [GCExtendedGamepad gamepadForController:tweak];

    return [tweak autorelease];
}

- (void)dealloc
{
    [_controllerPausedHandler release];
    [_vendorName release];
    [_gamepad release];
    [_extendedGamepad release];
    [_tweakElements release];
    [super dealloc];
}

- (void)tweakUpdateButtons:(const MFiWInputStatePacket*)data;
{   
    const float* buf = (float*)data;
    for(GCControllerElement* i in self.tweakElements)
        buf += [i tweakSetValues:buf];
}

- (void)setPlayerIndex:(NSInteger)index
{
    _playerIndex = index;
    MFiWrapperFrontend::SetControllerIndex(self.tweakHandle, index);
}

@end

/*************/
/* GCGamepad */
/*************/
@implementation GCGamepad

+ (GCGamepad*)gamepadForController:(GCController*)controller
{
    GCGamepad* tweak = [GCGamepad new];
    tweak.controller = controller;

    tweak.dpad = controller.tweakElements[MFi_DPad];
    tweak.buttonA = controller.tweakElements[MFi_A];
    tweak.buttonB = controller.tweakElements[MFi_B];
    tweak.buttonX = controller.tweakElements[MFi_X];
    tweak.buttonY = controller.tweakElements[MFi_Y];
    
    tweak.leftShoulder = controller.tweakElements[MFi_LeftShoulder];
    tweak.rightShoulder = controller.tweakElements[MFi_RightShoulder];
    
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];    
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
    
    tweak.dpad = controller.tweakElements[MFi_DPad];
    tweak.leftThumbstick = controller.tweakElements[MFi_LeftThumbstick];
    tweak.rightThumbstick = controller.tweakElements[MFi_RightThumbstick];
    
    tweak.buttonA = controller.tweakElements[MFi_A];
    tweak.buttonB = controller.tweakElements[MFi_B];
    tweak.buttonX = controller.tweakElements[MFi_X];
    tweak.buttonY = controller.tweakElements[MFi_Y];
    
    tweak.leftShoulder = controller.tweakElements[MFi_LeftShoulder];
    tweak.rightShoulder = controller.tweakElements[MFi_RightShoulder];
    tweak.leftTrigger = controller.tweakElements[MFi_LeftTrigger];
    tweak.rightTrigger = controller.tweakElements[MFi_RightTrigger];
    
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];    
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
@implementation GCControllerElement

- (void)tweakSetValue:(float)value
{

}

- (uint32_t)tweakSetValues:(const float*)value
{
    return 0;
}

@end

/***************************/
/* GCControllerButtonInput */
/***************************/
@implementation GCControllerButtonInput

+ (GCControllerButtonInput*)buttonWithParent:(GCControllerElement*)parent analog:(BOOL)isAnalog
{
    GCControllerButtonInput* tweak = [GCControllerButtonInput new];
    tweak.collection = parent;
    tweak.analog = isAnalog;
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [super dealloc];
}

- (void)tweakSetValue:(float)value
{
    if (value != self.value)
    {
        self.value = value;
        
        // NOTE: Docs say this should be .25f
        self.pressed = value >= 0.0f;

        if (self.valueChangedHandler)
            self.valueChangedHandler(self, self.value, self.pressed);
    }
}

- (uint32_t)tweakSetValues:(const float*)values
{
    [self tweakSetValue:*values];
    return 1;
}

@end

/*************************/
/* GCControllerAxisInput */
/*************************/
@implementation GCControllerAxisInput

+ (GCControllerAxisInput*)axisWithParent:(GCControllerElement*)parent analog:(BOOL)isAnalog
{
    GCControllerAxisInput* tweak = [GCControllerAxisInput new];
    tweak.collection = parent;
    tweak.analog = isAnalog;
    return [tweak autorelease];
}

- (void)dealloc
{
    [_valueChangedHandler release];
    [super dealloc];
}

- (void)tweakSetValue:(float)value
{
    if (value != self.value)
    {
        self.value = value;
        
        if (self.valueChangedHandler)
            self.valueChangedHandler(self, self.value);
    }
}

- (uint32_t)tweakSetValues:(const float*)values
{
    [self tweakSetValue:*values];
    return 1;
}

@end

/****************************/
/* GCControllerDirectionPad */
/****************************/
@implementation GCControllerDirectionPad

+ (GCControllerDirectionPad*)dpadWithParent:(GCControllerElement*)parent analog:(BOOL)isAnalog
{
    GCControllerDirectionPad* tweak = [GCControllerDirectionPad new];
    tweak.collection = parent;
    tweak.analog = isAnalog;

    tweak.xAxis = [GCControllerAxisInput axisWithParent:tweak analog:isAnalog];
    tweak.yAxis = [GCControllerAxisInput axisWithParent:tweak analog:isAnalog];

    tweak.up = [GCControllerButtonInput buttonWithParent:tweak analog:isAnalog];
    tweak.down = [GCControllerButtonInput buttonWithParent:tweak analog:isAnalog];
    tweak.left = [GCControllerButtonInput buttonWithParent:tweak analog:isAnalog];
    tweak.right = [GCControllerButtonInput buttonWithParent:tweak analog:isAnalog];

    return [tweak autorelease];
}

- (void)delloc
{
    [_valueChangedHandler release];
    [_xAxis release];
    [_yAxis release];
    [_up release];
    [_down release];
    [_left release];
    [_right release];
    [super dealloc];
}

- (uint32_t)tweakSetValues:(const float*)values
{
    if (self.xAxis.value == values[0] && self.yAxis.value == values[1])
        return 2;

    [self.xAxis tweakSetValue:values[0]];
    [self.left  tweakSetValue:(values[0] < 0.0f) ? fabsf(values[0]) : 0.0f];
    [self.right tweakSetValue:(values[0] > 0.0f) ?       values[0]  : 0.0f];
    
    [self.yAxis tweakSetValue:values[1]];
    [self.up    tweakSetValue:(values[1] < 0.0f) ? fabsf(values[1]) : 0.0f];
    [self.down  tweakSetValue:(values[1] > 0.0f) ?       values[1]  : 0.0f];

    if (self.valueChangedHandler)
        self.valueChangedHandler(self, values[0], values[1]);

    return 2;
}

@end
