#pragma once

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

