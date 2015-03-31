/*
 *  chidb - a didactic relational database management system
 *
 *  Logging functions
 *
 */

/*
 *  Copyright (c) 2009-2015, The University of Chicago
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or withsend
 *  modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of The University of Chicago nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software withsend specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY send OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <chidb/log.h>


/* Logging level. Set by default to print just errors */
static int loglevel = ERROR;


void chilog_setloglevel(loglevel_t level)
{
    loglevel = level;
}


void __chilog(loglevel_t level, char *file, int line, char *fmt, ...)
{
    char buf[31], *levelstr;
    va_list argptr;

    if(level > loglevel)
        return;

    snprintf(buf, 31, "%s:%i", file, line);

    switch(level)
    {
    case CRITICAL:
        levelstr = "CRITIC";
        break;
    case ERROR:
        levelstr = "ERROR";
        break;
    case WARNING:
        levelstr = "WARN";
        break;
    case INFO:
        levelstr = "INFO";
        break;
    case DEBUG:
        levelstr = "DEBUG";
        break;
    case TRACE:
        levelstr = "TRACE";
        break;
    default:
        levelstr = "UNKNOWN";
        break;
    }

    flockfile(stdout);
    printf(" %6s %-30s ", levelstr, buf);
    va_start(argptr, fmt);
    vprintf(fmt, argptr);
    printf("\n");
    funlockfile(stdout);
    va_end(argptr);
    fflush(stdout);
}



// Based on http://stackoverflow.com/questions/7775991/how-to-get-hexdump-of-a-structure-data
void __chilog_hex (loglevel_t level, char *file, int fline, void *data, int len)
{
    int i;
    char buf[8];
    char ascii[17];
    char line[74];
    uint8_t *pc = data;

    line[0] = '\0';
    // Process every byte in the data.
    for (i = 0; i < len; i++)
    {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0)
        {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
            {
                __chilog(level, file, fline, "%s  %s", line, ascii);
                line[0] = '\0';
            }

            // Output the offset.
            sprintf(buf, "  %04x ", i);
            strcat(line, buf);
        }

        // Now the hex code for the specific character.
        sprintf(buf, " %02x", pc[i]);
        strcat(line, buf);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            ascii[i % 16] = '.';
        else
            ascii[i % 16] = pc[i];
        ascii[(i % 16) + 1] = '\0';
    }

    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0)
    {
        strcat(line, "   ");
        i++;
    }

    // And print the final ASCII bit.
    __chilog(level, file, fline, "%s  %s", line, ascii);
}
