/*************************************************************************
* This is error and debug message handling of StreamDevice.
* Please see ../docs/ for detailed documentation.
*
* (C) 2005 Dirk Zimoch (dirk.zimoch@psi.ch)
*
* This file is part of StreamDevice.
*
* StreamDevice is free software: You can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* StreamDevice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with StreamDevice. If not, see https://www.gnu.org/licenses/.
*************************************************************************/

#ifndef StreamError_h
#define StreamError_h

#include <stdarg.h>
#include <stddef.h>

#include "messageEngine.h"

#ifndef __GNUC__
#define __attribute__(x)
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern StreamErrorEngine* pErrEngine;

extern int streamDebug;
extern int streamError;
extern int streamDebugColored;
extern int streamMsgTimeStamped;
#ifdef __cplusplus
}
#endif

extern void (*StreamPrintTimestampFunction)(char* buffer, size_t size);
extern const char* (*StreamGetThreadNameFunction)(void);

void StreamError(int line, const char* file, const char* fmt, ...)
__attribute__((__format__(__printf__,3,4)));

void StreamVError(int line, const char* file,
                  ErrorCategory category,
                  const char* fmt, va_list args)
__attribute__((__format__(__printf__,4,0)));

void StreamError(const char* fmt, ...)
__attribute__((__format__(__printf__,1,2)));

// Need to place a useless bool parameter to avoid overload ambiguity
void StreamError(bool useless, int line, const char* file,
                 ErrorCategory category,
                 const char* fmt, ...)
__attribute__ ((format(printf,5,6)));

// Need to place a useless bool parameter to avoid overload ambiguity
void StreamError(bool useless, ErrorCategory category, 
                 const char* fmt, ...)
__attribute__ ((format(printf,3,4)));

inline void StreamVError(const char* fmt, va_list args)
{
    StreamVError(0, NULL, CAT_NONE, fmt, args); 
}

class StreamDebugClass
{
    const char* file;
    int line;
public:
    StreamDebugClass(const char* file, int line) :
        file(file), line(line) {}
    int print(const char* fmt, ...)
        __attribute__((__format__(__printf__,2,3)));
};

inline StreamDebugClass
StreamDebugObject(const char* file, int line)
{ return StreamDebugClass(file, line); }

#define error StreamError
#define debug (!streamDebug)?0:StreamDebugObject(__FILE__,__LINE__).print
#define debug2 (streamDebug<2)?0:StreamDebugObject(__FILE__,__LINE__).print

/*
 * ANSI escape sequences for terminal output
 */
enum AnsiMode { ANSI_REVERSE_VIDEO, ANSI_NOT_REVERSE_VIDEO, ANSI_BG_WHITE,
                ANSI_RESET, ANSI_RED_BOLD };
extern const char* ansiEscape(AnsiMode mode);

#endif
