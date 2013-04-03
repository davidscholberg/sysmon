/*
    Copyright 2013 David Scholberg <recombinant.vector@gmail.com>

    This file is part of sysmon.

    sysmon is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    sysmon is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sysmon.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "include/error.h"
#include "include/timeout.h"

void timeout( time_t seconds, long nanoseconds)
{
    struct timespec req = { seconds, nanoseconds };
    struct timespec rem;
    int err;
    while( ( err = clock_nanosleep( CLOCK_MONOTONIC, 0, &req, &rem ) ) != 0 )
    {
        if( err == EINTR )
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
        else
        {
            die_errnum( "clock_nanosleep failed", err );
        }
    }
}
