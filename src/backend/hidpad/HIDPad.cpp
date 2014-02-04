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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "HIDPad.h"
#include "HIDManager.h"
#include "backend.h"

HIDPad::Interface::Interface(HIDManager::Connection* aConnection) :
    handle(0), connection(aConnection)
{
}

HIDPad::Interface::~Interface()
{
    MFiWrapperBackend::DetachController(this);
}

float HIDPad::Interface::CalculateAxis(int32_t aValue, const int32_t aCalibration[4])
{
    assert(aCalibration[0] < aCalibration[1]);
    assert(aCalibration[1] <= aCalibration[2]);
    assert(aCalibration[2] < aCalibration[3]);

    // Handle out of range cases
    if (aValue < aCalibration[0])   return -1.0f;
    if (aValue > aCalibration[3])   return 1.0f;
    
    // Check if it's inside the dead zone
    if (aValue >= aCalibration[1] && aValue <= aCalibration[2])
        return 0.0f;
        
    if (aValue < aCalibration[1])
    {
        float val = aValue - aCalibration[0];
        float div = aCalibration[1] - aCalibration[0];
        return 0.0f - (val / div);
    }
    else
    {
        float val = aValue - aCalibration[2];
        float div = aCalibration[3] - aCalibration[2];
        return (val / div);
    }
}

void HIDPad::Interface::FinalizeConnection()
{
    handle = MFiWrapperBackend::AttachController(this);
}

//

HIDPad::Interface* HIDPad::Connect(const char* aName, HIDManager::Connection* aConnection)
{            
    if (strstr(aName, "PLAYSTATION(R)3 Controller"))
        return new Playstation3(aConnection);
    else if (strstr(aName, "Nintendo RVL-CNT-01"))
        return new HIDPad::WiiMote(aConnection);
    else /* DUMMY */
        return new Interface(aConnection);            
}
