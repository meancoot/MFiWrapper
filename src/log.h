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
#include <asl.h>

namespace MFiWrapperCommon {

struct Logger
{
    Logger(const std::string& aHeader) : header(aHeader) { }


    void DoPrint(const char* aMessage, va_list ap)
    {
        char buffer[1024];
        snprintf(buffer, 1024, "(MFiW: %s) %s", header.c_str(), aMessage);

        aslmsg msg = asl_new(ASL_TYPE_MSG);
        asl_set(msg, ASL_KEY_READ_UID, "-1");    
        asl_vlog(0, msg, ASL_LEVEL_NOTICE, buffer, ap);
        asl_free(msg);
    }

    void Error(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 0
        va_list args;
        va_start(args, aMessage);
        DoPrint(aMessage, args);
        va_end(args);
        #endif 
    }

    void Warning(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 1
        va_list args;
        va_start(args, aMessage);
        DoPrint(aMessage, args);
        va_end(args);
        #endif
    }
        
    void Verbose(const char* aMessage, ...)
    {
        #if LOG_LEVEL > 2
        va_list args;
        va_start(args, aMessage);
        DoPrint(aMessage, args);
        va_end(args);
        #endif 
    }
    
private:
    std::string header;
};

#undef MFIW_DO_PRINT

}
