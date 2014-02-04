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

#include <stdint.h>
#include "backend.h"
#include "wiimote.h"

namespace HIDManager
{
    class Connection;
}

namespace HIDPad
{
    class Interface
    {
        public:
            Interface(const Interface&) = delete;

            Interface(HIDManager::Connection* aConnection);
            virtual ~Interface();

            uint32_t GetHandle() const { return handle; }

            HIDManager::Connection* GetConnection() { return connection; }

            virtual void SetPlayerIndex(int32_t aIndex) { }

            virtual void HandlePacket(uint8_t *aData, uint16_t aSize) { }
            
            virtual const char* GetVendorName() const { return "Unknown Vendor"; }
            virtual uint32_t GetPresentControls() const { return 0xFFFFFFFF; }
            virtual uint32_t GetAnalogControls() const { return 0; }

            // aCalibration Values (both ranges are inclusive):
            // 0 -> 3: Total analog range
            // 1 -> 2: Dead zone
            // Must satisfy 0 < 1 <= 2 < 3
            static float CalculateAxis(int32_t aValue, const int32_t aCalibration[4]);
            
        protected:
            void FinalizeConnection();
        
            uint32_t handle;
            HIDManager::Connection* connection;
    };
    
    class Playstation3 : public Interface
    {
        public:
            Playstation3(HIDManager::Connection* aConnection);
            virtual void SetPlayerIndex(int32_t aIndex);
            virtual void HandlePacket(uint8_t *aData, uint16_t aSize);

            virtual const char* GetVendorName() const;
            virtual uint32_t GetPresentControls() const;
            virtual uint32_t GetAnalogControls() const;
            
        private:
            void SetReport();
        
            bool pauseHeld;
            uint8_t ledByte;
            bool needSetReport;
    };

    class WiiMote : public Interface
    {
        public:
            WiiMote(HIDManager::Connection* aConnection);             
            virtual void SetPlayerIndex(int32_t aIndex);
            virtual void HandlePacket(uint8_t *aData, uint16_t aSize);
            
            virtual const char* GetVendorName() const;

        private:
            void ProcessButtons();

            wiimote_t device;
    };

    Interface* Connect(const char* aName, HIDManager::Connection* aConnection);
}
