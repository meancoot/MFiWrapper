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
#include <assert.h>
#include <set>

#include <CoreFoundation/CFRunLoop.h>
#include <IOKit/hid/IOHIDManager.h>
#include "HIDManager.h"
#include "HIDPad.h"

namespace HIDManager
{
    IOHIDManagerRef g_hid_manager;
    static void append_matching_dictionary(CFMutableArrayRef array, uint32_t page, uint32_t use);
    static void DeviceRemoved(void* context, IOReturn result, void* sender);
    static void DeviceReport(void* context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength);
    static void DeviceAttached(void* context, IOReturn result, void* sender, IOHIDDeviceRef device);

    class Connection
    {   public:
        HIDPad::Interface* hidpad;
        IOHIDDeviceRef device;
        uint8_t data[2048];
    
        Connection() : hidpad(0), device(0)
        {
            memset(data, 0, sizeof(data));
        }
    
        ~Connection()
        {
            if (device)
                IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
            delete hidpad;
        }    
    };
        
    void StartUp()
    {   
        g_hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

        CFMutableArrayRef matcher = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        append_matching_dictionary(matcher, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
        append_matching_dictionary(matcher, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);

        IOHIDManagerSetDeviceMatchingMultiple(g_hid_manager, matcher);
        CFRelease(matcher);

        IOHIDManagerRegisterDeviceMatchingCallback(g_hid_manager, DeviceAttached, 0);
        IOHIDManagerScheduleWithRunLoop(g_hid_manager, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);

        IOHIDManagerOpen(g_hid_manager, kIOHIDOptionsTypeNone);
    }
    
    void ShutDown()
    {
        IOHIDManagerClose(g_hid_manager, kIOHIDOptionsTypeNone);
        IOHIDManagerUnscheduleFromRunLoop(g_hid_manager, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
      
        CFRelease(g_hid_manager);
    }
    
    void SetReport(Connection* aConnection, bool aFeature, uint8_t aID, uint8_t* aData, uint16_t aSize)
    {
        IOHIDDeviceSetReport(aConnection->device, aFeature ? kIOHIDReportTypeFeature : kIOHIDReportTypeOutput,
                             aID, aData + 1, aSize - 1);
    }

    void GetReport(Connection* aConnection, bool aFeature, uint8_t aID, uint8_t* aData, uint16_t aSize)
    {
        CFIndex size = aSize - 1;
        IOHIDDeviceGetReport(aConnection->device, aFeature ? kIOHIDReportTypeFeature : kIOHIDReportTypeInput,
                             aID, aData + 1, &size);
    }

    //
    
    static void append_matching_dictionary(CFMutableArrayRef array, uint32_t page, uint32_t use)
    {
        CFMutableDictionaryRef matcher = CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

        CFNumberRef pagen = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &page);
        CFDictionarySetValue(matcher, CFSTR(kIOHIDDeviceUsagePageKey), pagen);
        CFRelease(pagen);

        CFNumberRef usen = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &use);
        CFDictionarySetValue(matcher, CFSTR(kIOHIDDeviceUsageKey), usen);
        CFRelease(usen);

        CFArrayAppendValue(array, matcher);
        CFRelease(matcher);
    }

    // NOTE: I pieced this together through trial and error, any corrections are welcome
    static void DeviceRemoved(void* context, IOReturn result, void* sender)
    {
        Connection* connection = (Connection*)context;
        delete connection;
    }

    static void DeviceReport(void* context, IOReturn result, void *sender, IOHIDReportType type, uint32_t reportID, uint8_t *report, CFIndex reportLength)
    {
       Connection* connection = (Connection*)context;
       connection->hidpad->HandlePacket(connection->data, reportLength + 1);
    }

    static void DeviceAttached(void* context, IOReturn result, void* sender, IOHIDDeviceRef device)
    {
       Connection* connection = new Connection();
       connection->device = device;

       IOHIDDeviceOpen(device, kIOHIDOptionsTypeNone);
       IOHIDDeviceScheduleWithRunLoop(device, CFRunLoopGetCurrent(), kCFRunLoopCommonModes);
       IOHIDDeviceRegisterRemovalCallback(device, DeviceRemoved, connection);

       CFStringRef device_name_ref = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
       char device_name[1024];
       CFStringGetCString(device_name_ref, device_name, sizeof(device_name), kCFStringEncodingUTF8);

       connection->hidpad = HIDPad::Connect(device_name, connection);
       IOHIDDeviceRegisterInputReportCallback(device, connection->data + 1, sizeof(connection->data) - 1, DeviceReport, connection);
    }
}
