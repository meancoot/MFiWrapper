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

#include "HIDPad.h"
#include "backend.h"


HIDPad::Interface::Interface(HIDManager::Connection* aConnection) :
    playerIndex(-1), connection(aConnection)
{
    connection = aConnection;
}

HIDPad::Interface::~Interface()
{
    MFiWrapperBackend::DetachController(this);
}

void HIDPad::Interface::FinalizeConnection()
{
    MFiWrapperBackend::AttachController(this);
}
