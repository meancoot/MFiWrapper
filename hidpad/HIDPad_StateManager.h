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
