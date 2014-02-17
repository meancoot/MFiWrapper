#pragma once

#include <stdio.h>
#include <stdint.h>

class PacketLogger
{
    public:
        PacketLogger(const char* aPath = 0);
        ~PacketLogger();
        
        void LogWrite(const uint8_t* aData, uint32_t aSize);
        void LogRead(const uint8_t* aData, uint32_t aSize);

        void Flush();

    private:
        FILE* file;
};

