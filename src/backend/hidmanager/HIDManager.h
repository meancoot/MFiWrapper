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

#include <CoreFoundation/CFRunLoop.h>

#include <stdint.h>
#include <stdlib.h>

// This namespace (static class) wraps the management of HID connections.
// Whenever a HID device is connected HIDManager will call HIDPad::Connect
// to bind a connection to the frontend.

// Whenever a HID packet arrives for the device HIDManager should call the
// HandlePacket member function of the class returned by HIDPad::Connect.

// To finalize a disconnection HIDManager should delete the class returned
// by HIDPad::Connect.

// HIDManager must be thread safe.

namespace HIDManager
{
    class Connection;

    void StartUp();
    void ShutDown();

    void SendPacket(Connection* aConnection, uint8_t* aData, size_t aSize);

    void StartDeviceProbe();
    void StopDeviceProbe();
};

