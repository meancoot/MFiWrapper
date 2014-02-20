#pragma once

#include <stdint.h>
#include <string.h>

class BTAddress
{
public:
    BTAddress(const uint8_t aAddress[6])
    {
        memcpy(address, aAddress, 6);
    }
    
    bool operator== (const BTAddress& aOther) const
    {
        return memcmp(address, aOther.address, 6) == 0;
    }
    
    bool operator!= (const BTAddress& aOther) const
    {
        return memcmp(address, aOther.address, 6) != 0;
    }
    
    bool operator< (const BTAddress& aOther) const
    {
        return memcmp(address, aOther.address, 6) < 0;
    }
    
    bool operator> (const BTAddress& aOther) const
    {
        return memcmp(address, aOther.address, 6) > 0;    
    }

private:
    uint8_t address[6];
};

