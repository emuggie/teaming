#include "debugger.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

namespace DBG
{
    int printf (const char * fmt, ...){
        #ifdef DEBUG
        va_list argptr;
        va_start(argptr, fmt);
        ::printf("[PID:%d] ", getpid());
        int rt = ::vprintf(fmt, argptr);
        ::printf("\n");
        va_end(argptr);
        return rt;
        #else
        return 0;
        #endif
    }
} // namespace DBG