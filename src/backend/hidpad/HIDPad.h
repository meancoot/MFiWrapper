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

            HIDManager::Connection* GetConnection() { return connection; }

            virtual void SetPlayerIndex(int32_t aIndex) { playerIndex = aIndex; }
            virtual int GetPlayerIndex() const { return playerIndex; }

            virtual void HandlePacket(uint8_t *aData, uint16_t aSize) { }
            
            virtual const char* GetVendorName() const { return "Unknown Vendor"; }
            
        protected:
            void FinalizeConnection();

            int32_t playerIndex;
            HIDManager::Connection* connection;
    };
    
    class Playstation3 : public Interface
    {
        public:
            Playstation3(HIDManager::Connection* aConnection);
            virtual void SetPlayerIndex(int32_t aIndex);
            virtual void HandlePacket(uint8_t *aData, uint16_t aSize);
            virtual const char* GetVendorName() const;
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

    class Interface;
    Interface* Connect(const char* aName, HIDManager::Connection* aConnection);
}
