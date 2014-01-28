#include <stdint.h>
#include <string.h>

#include "HIDPad.h"

#include "../HIDManager.h"
#include <stdio.h>

HIDPad::Interface* HIDPad::Connect(const char* aName, HIDManager::Connection* aConnection)
{            
    if (strstr(aName, "PLAYSTATION(R)3 Controller"))
        return new Playstation3(aConnection);
    else if (strstr(aName, "Nintendo RVL-CNT-01"))
        return new HIDPad::WiiMote(aConnection);
    else /* DUMMY */
        return new Interface(aConnection);            
}
