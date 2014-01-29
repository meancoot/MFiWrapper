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
#include <pthread.h>
#include <assert.h>
#include <set>

#include <CoreFoundation/CFRunLoop.h>
#include <IOKit/hid/IOHIDManager.h>
#include "HIDManager.h"
#include "HIDPad.h"

namespace HIDManager
{
#ifndef NDEBUG
    #define ASSERT_THREAD assert(pthread_equal(managerThread, pthread_self()))
#else
    #define ASSERT_THREAD
#endif

    pthread_t managerThread;
    CFRunLoopRef managerRunLoop;
    IOHIDManagerRef g_hid_manager;

    void* ManagerThreadFunction(void* unused);

    class Connection
    {   public:
        HIDPad::Interface* hidpad;
        IOHIDDeviceRef device;
        uint8_t data[2048];
    
        Connection() : hidpad(0), device(0)
        {
            ASSERT_THREAD;
            memset(data, 0, sizeof(data));
        }
    
        ~Connection()
        {
            ASSERT_THREAD;      
            if (device)
                IOHIDDeviceClose(device, kIOHIDOptionsTypeNone);
            delete hidpad;
        }    
    };
        
    void StartUp()
    {
        if (!managerThread)
            pthread_create(&managerThread, 0, ManagerThreadFunction, 0);  
    }
    
    void ShutDown()
    {

    }
    
    void SendPacket(Connection* aConnection, uint8_t* aData, size_t aSize)
    {
        IOHIDDeviceSetReport(aConnection->device, kIOHIDReportTypeOutput, 0x01, aData + 1, aSize - 1);
    
    
/*        if (btstackRunLoop == CFRunLoopGetCurrent())
            bt_send_l2cap(aConnection->channels[0], aData, aSize);
        else if (btstackRunLoop)
        {
            // (TODO) THREADING: What if aConnection is deleted before
            //                   the block is run? Maybe the block can
            //                   check if aConnection is present in the
            //                   Connections set before running.
            uint8_t* data = new uint8_t[aSize];
            memcpy(data, aData, aSize);

            CFRunLoopPerformBlock(btstackRunLoop, kCFRunLoopCommonModes, ^{
                bt_send_l2cap(aConnection->channels[0], data, aSize);
                delete[] data;
            });
            CFRunLoopWakeUp(btstackRunLoop);
        }*/
    }
    
    void StartDeviceProbe()
    {
        // Not supported:
        //     Any devices must connect to the Mac directly. Fortunately OS X
        //     will perma-pair itself with both DualShock 3 and WiiMotes when
        //     connected.
    }
    
    void StopDeviceProbe()
    {
        // Not supported
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
       IOHIDDeviceScheduleWithRunLoop(device, managerRunLoop, kCFRunLoopCommonModes);
       IOHIDDeviceRegisterRemovalCallback(device, DeviceRemoved, connection);

       CFStringRef device_name_ref = (CFStringRef)IOHIDDeviceGetProperty(device, CFSTR(kIOHIDProductKey));
       char device_name[1024];
       CFStringGetCString(device_name_ref, device_name, sizeof(device_name), kCFStringEncodingUTF8);

       connection->hidpad = HIDPad::Connect(device_name, connection);
       IOHIDDeviceRegisterInputReportCallback(device, connection->data + 1, sizeof(connection->data) - 1, DeviceReport, connection);
    }
    
    void* ManagerThreadFunction(void* unused)
    {
        ASSERT_THREAD;
        managerRunLoop = CFRunLoopGetCurrent();
        
        g_hid_manager = IOHIDManagerCreate(kCFAllocatorDefault, kIOHIDOptionsTypeNone);

        CFMutableArrayRef matcher = CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);
        append_matching_dictionary(matcher, kHIDPage_GenericDesktop, kHIDUsage_GD_Joystick);
        append_matching_dictionary(matcher, kHIDPage_GenericDesktop, kHIDUsage_GD_GamePad);

        IOHIDManagerSetDeviceMatchingMultiple(g_hid_manager, matcher);
        CFRelease(matcher);

        IOHIDManagerRegisterDeviceMatchingCallback(g_hid_manager, DeviceAttached, 0);
        IOHIDManagerScheduleWithRunLoop(g_hid_manager, managerRunLoop, kCFRunLoopCommonModes);

        IOHIDManagerOpen(g_hid_manager, kIOHIDOptionsTypeNone);

        CFRunLoopRun();

        IOHIDManagerClose(g_hid_manager, kIOHIDOptionsTypeNone);
        IOHIDManagerUnscheduleFromRunLoop(g_hid_manager, managerRunLoop, kCFRunLoopCommonModes);
      
        CFRelease(g_hid_manager);
        return 0;    
    }
}
