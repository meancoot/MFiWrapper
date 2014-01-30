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

#include "hidpad/HIDPad.h"

void attach_tweak_controller(HIDPad::Interface* aInterface);
void detach_tweak_controller(HIDPad::Interface* aInterface);

enum MFiButtons
{
    MFi_A, MFi_B, MFi_X, MFi_Y, MFi_LeftShoulder, MFi_RightShoulder,
    MFi_LeftTrigger, MFi_RightTrigger, MFi_Up, MFi_Down, MFi_Left, MFi_Right,
    MFi_LeftUp, MFi_LeftDown, MFi_LeftLeft, MFi_LeftRight,
    MFi_RightUp, MFi_RightDown, MFi_RightLeft, MFi_RightRight,
    MFi_LastButton
};

#ifdef __OBJC__

#include <Foundation/Foundation.h>

#define GCController                GCControllerTweak
#define GCGamepad                   GCGamepadTweak
#define GCExtendedGamepad           GCExtendedGamepadTweak
#define GCControllerElement         GCControllerElementTweak
#define GCControllerButtonInput     GCControllerButtonInputTweak
#define GCControllerAxisInput       GCControllerAxisInputTweak
#define GCControllerDirectionPad    GCControllerDirectionPadTweak
#define GCGamepadSnapshot           GCGamepadSnapshotTweak
#define GCExtendedGamepadSnapshot   GCExtendedGamepadSnapshotTweak

@class GCController;
@class GCGamepad;
@class GCExtendedGamepad;
@class GCControllerElement;
@class GCControllerButtonInput;
@class GCControllerAxisInput;
@class GCControllerDirectionPad;
@class GCGamepadSnapshot;
@class GCExtendedGamepadSnapshot;

enum { GCControllerPlayerIndexUnset = -1 };

/****************/
/* GCController */
/****************/
@interface GCController : NSObject

@property (copy) void (^controllerPausedHandler)(GCController *controller);
@property (readonly, copy) NSString *vendorName;
@property (getter = isAttachedToDevice) BOOL attachedToDevice;
@property (nonatomic) NSInteger playerIndex;
@property (retain) GCGamepad *gamepad;
@property (retain) GCExtendedGamepad *extendedGamepad;

@property(nonatomic, retain) NSMutableArray* tweakButtons;
@property(nonatomic, retain) NSMutableArray* tweakAxis;
@property(nonatomic) HIDPad::Interface* tweakHIDPad;
+ (GCController*)controllerForHIDPad:(HIDPad::Interface*)hidpad;

@end

/*************/
/* GCGamepad */
/*************/
@interface GCGamepad : NSObject
@property (assign) GCController *controller;

typedef void (^GCGamepadValueChangedHandler)(GCGamepad *gamepad, GCControllerElement *element);
@property (copy) GCGamepadValueChangedHandler valueChangedHandler;

- (GCGamepadSnapshot *)saveSnapshot;

@property (retain) GCControllerDirectionPad *dpad;
@property (assign) GCControllerButtonInput *buttonA;
@property (assign) GCControllerButtonInput *buttonB;
@property (assign) GCControllerButtonInput *buttonX;
@property (assign) GCControllerButtonInput *buttonY;
@property (assign) GCControllerButtonInput *leftShoulder;
@property (assign) GCControllerButtonInput *rightShoulder;

+ (GCGamepad*)gamepadForController:(GCController*)controller;

@end

/*********************/
/* GCExtendedGamepad */
/*********************/
@interface GCExtendedGamepad : NSObject

@property (assign) GCController *controller;

typedef void (^GCExtendedGamepadValueChangedHandler)(GCExtendedGamepad *gamepad, GCControllerElement *element);
@property (copy) GCExtendedGamepadValueChangedHandler valueChangedHandler;

- (GCExtendedGamepadSnapshot *)saveSnapshot;

@property (retain) GCControllerDirectionPad *dpad;
@property (retain) GCControllerDirectionPad *leftThumbstick;
@property (retain) GCControllerDirectionPad *rightThumbstick;

@property (assign) GCControllerButtonInput *buttonA;
@property (assign) GCControllerButtonInput *buttonB;
@property (assign) GCControllerButtonInput *buttonX;
@property (assign) GCControllerButtonInput *buttonY;
@property (assign) GCControllerButtonInput *leftShoulder;
@property (assign) GCControllerButtonInput *rightShoulder;
@property (assign) GCControllerButtonInput *leftTrigger;
@property (assign) GCControllerButtonInput *rightTrigger;

+ (GCExtendedGamepad*)gamepadForController:(GCController*)controller;

@end

/***********************/
/* GCControllerElement */
/***********************/
@interface GCControllerElement : NSObject
@property (assign) GCControllerElement *collection;
@property (getter = isAnalog) BOOL analog;

@property(nonatomic, assign) GCController* tweakController;
@property(nonatomic) NSInteger tweakIndex;

@end

/***************************/
/* GCControllerButtonInput */
/***************************/
@interface GCControllerButtonInput : GCControllerElement
typedef void (^GCControllerButtonValueChangedHandler)(GCControllerButtonInput *button, float value, BOOL pressed);
@property (copy) GCControllerButtonValueChangedHandler valueChangedHandler;
@property float value;
@property (getter = isPressed) BOOL pressed;

+ (GCControllerButtonInput*)buttonForController:(GCController*)controller index:(NSInteger)index;

@end

/*************************/
/* GCControllerAxisInput */
/*************************/
@interface GCControllerAxisInput : GCControllerElement
typedef void (^GCControllerAxisValueChangedHandler)(GCControllerAxisInput *axis, float value);
@property (copy) GCControllerAxisValueChangedHandler valueChangedHandler;
@property float value;

+ (GCControllerAxisInput*)axisForController:(GCController*)controller index:(NSInteger)index;

@end

/****************************/
/* GCControllerDirectionPad */
/****************************/
@interface GCControllerDirectionPad : GCControllerElement
typedef void (^GCControllerDirectionPadValueChangedHandler)(GCControllerDirectionPad *dpad, float xValue, float yValue);
@property (copy) GCControllerDirectionPadValueChangedHandler valueChangedHandler;
@property (assign) GCControllerAxisInput *xAxis;
@property (assign) GCControllerAxisInput *yAxis;
@property (assign) GCControllerButtonInput *up;
@property (assign) GCControllerButtonInput *down;
@property (assign) GCControllerButtonInput *left;
@property (assign) GCControllerButtonInput *right;

+ (GCControllerDirectionPad*)dpadForController:(GCController*)controller index:(NSInteger)index;

@end

/*********************/
/* GCGamepadSnapshot */
/*********************/
@interface GCGamepadSnapshot : GCGamepad
@property (copy) NSData *snapshotData;

- (instancetype)initWithSnapshotData:(NSData *)data;
- (instancetype)initWithController:(GCController *)controller snapshotData:(NSData *)data;

@end

/*****************************/
/* GCExtendedGamepadSnapshot */
/*****************************/
@interface GCExtendedGamepadSnapshot : GCExtendedGamepad
@property (copy) NSData *snapshotData;

- (instancetype)initWithSnapshotData:(NSData *)data;
- (instancetype)initWithController:(GCController *)controller snapshotData:(NSData *)data;

@end

#endif
