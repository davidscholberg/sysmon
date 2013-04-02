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
#include <time.h>
#include "multithread.h"
#include "error.h"

#define BUFFER_LEN  30

void *date_mon( void *thread_id )
{
    // get time struct
    time_t time_in_seconds = time( NULL );
    if( time_in_seconds == -1 )
    {
        die( "time in date_mon failed" );
    }
    struct tm broken_down_time;
    if( localtime_r( &time_in_seconds, &broken_down_time ) == NULL )
    {
        die_sans_errno( "localtime_r in date_mon failed" );
    }
    
    char out_buffer[BUFFER_LEN];

    // write time to output buffer
    int bytes_written = strftime( out_buffer, BUFFER_LEN, "%a, %b %e %l:%M %p",
        &broken_down_time );
    if ( bytes_written == 0 )
    {
        die_sans_errno( "strftime in date_mon failed" );
    }
    
    multithread_write( out_buffer, ( int * ) thread_id );
    
    return NULL;
}
