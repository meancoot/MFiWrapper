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

#include <string>

namespace MFiWrapperCommon {

#define MFIW_DO_PRINT              \
{                                  \
  printf("(%s):", header.c_str()); \
  va_list args;                    \
  va_start(args, aMessage);        \
  vprintf(aMessage, args);         \
  va_end(args);                    \
  printf("\n");                    \
}

struct Logger
{
    Logger(const std::string& aHeader) : header(aHeader) { }

    void Error(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 0
        MFIW_DO_PRINT;
        #endif 
    }

    void Warning(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 1
        MFIW_DO_PRINT;
        #endif
    }
        
    void Verbose(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 2
        MFIW_DO_PRINT;
        #endif 
    }
    
private:
    std::string header;
};

#undef MFIW_DO_PRINT

}
