#pragma once

#include <stdint.h>
#include "wiimote.h"

namespace HIDManager
{
    class Connection;
}

#include "HIDPad_StateManager.h"

namespace HIDPad
{
    class Interface
    {
        public:
            Interface(const Interface&) = delete;
        
            Interface(HIDManager::Connection* aConnection);
            virtual ~Interface();
            
            void SetStateManager(StateManager* aManager) { stateManager = aManager; }        
            StateManager* GetStateManager() { return stateManager; }

            HIDManager::Connection* GetConnection() { return connection; }

            virtual void SetPlayerIndex(int32_t aIndex) { playerIndex = aIndex; }
            virtual int GetPlayerIndex() const { return playerIndex; }

            virtual void HandlePacket(uint8_t *aData, uint16_t aSize) { }
            
        protected:
            void FinalizeConnection();

            int32_t playerIndex;
            HIDManager::Connection* connection;
            
            StateManager* stateManager;
    };
    
    class Playstation3 : public Interface
    {
        public:
            Playstation3(HIDManager::Connection* aConnection);
            virtual void SetPlayerIndex(int32_t aIndex);
            virtual void HandlePacket(uint8_t *aData, uint16_t aSize);

        private:
            char data[512];
    };

    class WiiMote : public Interface
    {
        public:
            WiiMote(HIDManager::Connection* aConnection);             
            virtual void SetPlayerIndex(int32_t aIndex);
            virtual void HandlePacket(uint8_t *aData, uint16_t aSize);

        private:
            void ProcessButtons();

            wiimote_t device;
    };

    class Interface;
    Interface* Connect(const char* aName, HIDManager::Connection* aConnection);
}
