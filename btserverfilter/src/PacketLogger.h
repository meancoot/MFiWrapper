#pragma once

#include <stdio.h>
#include <stdint.h>

class PacketLogger
{
    public:
        PacketLogger(const char* aIP, unsigned short aPort);
        ~PacketLogger();

        void LogWrite(const uint8_t* aData, uint32_t aSize);
        void LogRead(const uint8_t* aData, uint32_t aSize);

        void Flush();

    private:
        template <typename T> void Write(T aValue)
        {
            if (socket < 0 || write(socket, &aValue, sizeof(aValue)) < 0)
            {
                socket = -1;
            }
        }


        int socket;
};

