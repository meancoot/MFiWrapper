#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "PacketLogger.h"

PacketLogger::PacketLogger(const char* aIP, unsigned short aPort) : socket(-1)
{
    if (!aIP || !aPort)
    {
        return;
    }

    if ((socket = ::socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        socket = -1;
        return;
    }

    sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family      = AF_INET;
    address.sin_port        = htons(aPort);
    address.sin_addr.s_addr = inet_addr(aIP);

    // Connect without blocking
    fd_set set;
    FD_ZERO(&set);
    FD_SET(socket, &set);

    fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) | O_NONBLOCK);

    if (connect(socket, (sockaddr*)&address, sizeof(sockaddr_in)) < 0 &&
        errno != EINPROGRESS)
    {
        close(socket);
        socket = -1;
        return;
    }

    timeval timeout = { 2, 0 };
    select(socket + 1, 0, &set, 0, &timeout);

    fcntl(socket, F_SETFL, fcntl(socket, F_GETFL, 0) & ~O_NONBLOCK);

    // Disable SIGPIPE for socket
    int nosigpipe = 1;
    setsockopt(socket, SOL_SOCKET, SO_NOSIGPIPE, &nosigpipe, sizeof(int));
}

PacketLogger::~PacketLogger()
{
    if (socket < 0)
    {
        return;
    }

    close(socket);
}

void PacketLogger::LogWrite(const uint8_t* aData, uint32_t aSize)
{
    if (socket < 0)
    {
        return;
    }

    Write<uint32_t>(aSize + 8);
    Write<uint32_t>(0UL);
    Write<uint32_t>(0UL);
    Write<uint8_t> ((aData[0] == 1) ? 0 : 2);
    if (write(socket, aData + 1, aSize - 1) < 0)
    {
        socket = -1;
    }
}

void PacketLogger::LogRead(const uint8_t* aData, uint32_t aSize)
{
    if (socket < 0)
    {
        return;
    }

    Write<uint32_t>(aSize + 8);
    Write<uint32_t>(0UL);
    Write<uint32_t>(0UL);
    Write<uint8_t> ((aData[0] == 4) ? 1 : 3);
    if (write(socket, aData + 1, aSize - 1) < 0)
    {
        socket = -1;
    }
}

void PacketLogger::Flush()
{
}
