#include "PacketLogger.h"

template <typename T> void Write(FILE* aFile, T aValue)
{
    fwrite(&aValue, 1, sizeof(T), aFile);
}

PacketLogger::PacketLogger(const char* aPath) : file(0)
{
    if (aPath)
    {
        file = fopen(aPath, "w");
    }
}

PacketLogger::~PacketLogger()
{
    if (file)
    {
        fclose(file);
    }
}

void PacketLogger::LogWrite(const uint8_t* aData, uint32_t aSize)
{
    if (!file)
    {
        return;
    }

    Write<uint32_t>(file, aSize + 8);
    Write<uint32_t>(file, 0UL);
    Write<uint32_t>(file, 0UL);
    Write<uint8_t> (file, (aData[0] == 1) ? 0 : 2);
    fwrite(aData + 1, 1, aSize - 1, file);
    fflush(file);    
}

void PacketLogger::LogRead(const uint8_t* aData, uint32_t aSize)
{
    if (!file)
    {
        return;
    }

    Write<uint32_t>(file, aSize + 8);
    Write<uint32_t>(file, 0UL);
    Write<uint32_t>(file, 0UL);
    Write<uint8_t> (file, (aData[0] == 4) ? 1 : 3);
    fwrite(aData + 1, 1, aSize - 1, file);
    fflush(file);   
}

void PacketLogger::Flush()
{
    if (file)
    {
        fflush(file);
    }
}
