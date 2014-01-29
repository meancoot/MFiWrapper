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
#include <string.h>

namespace HIDPad
{
    class StateManager
    {
        public:
            StateManager()
            {
                memset(buttons, 0, sizeof(buttons));
                memset(axis, 0, sizeof(axis));
            }
            
            // Buttons
            virtual void ButtonHasChanged(uint32_t aIndex) = 0;
            
            float GetButton(uint32_t aIndex)
            {
                return (aIndex < 20) ? buttons[aIndex] : 0.0f;
            }
            
            void SetButton(uint32_t aIndex, float aValue)
            {
                if (aIndex < 20 && aValue != buttons[aIndex])
                {
                    buttons[aIndex] = aValue;
                    ButtonHasChanged(aIndex);
                }
            }
        
        
            // Axis
            virtual void AxisHasChanged(uint32_t aIndex) = 0;
            
            float GetAxis(uint32_t aIndex)
            {
                return (aIndex < 6) ? axis[aIndex] : 0.0f;
            }
            
            void SetAxis(uint32_t aIndex, float aValue)
            {
                if (aIndex < 6 && aValue != axis[aIndex])
                {
                    axis[aIndex] = aValue;
                    AxisHasChanged(aIndex);
                }
            }
        
        private:
            float buttons[20];
            float axis[6];
    };
}
