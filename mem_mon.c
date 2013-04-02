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
#include "multithread.h"
#include "error.h"

#define BUFFER_LEN  20

#define MEMTOTAL        0
#define MEMFREE         1
#define BUFFERS         2
#define CACHED          3
#define MEM_STAT_LEN    4

void *mem_mon( void *thread_id )
{
    // open /proc/meminfo and read values from it
    FILE *procmem = fopen( "/proc/meminfo", "r" );
    if( procmem == NULL ) 
    {
        die( "couldn't open /proc/meminfo in mem_mon" );
    }
    unsigned long int mem_stats[MEM_STAT_LEN];
    int values_read = fscanf( procmem,
        "MemTotal: %lu kB\n"
        "MemFree: %lu kB\n"
        "Buffers: %lu kB\n"
        "Cached: %lu kB\n",
        mem_stats + MEMTOTAL,
        mem_stats + MEMFREE,
        mem_stats + BUFFERS,
        mem_stats + CACHED );
    if( values_read != MEM_STAT_LEN )
    {
        if( ferror( procmem ) )
        {
            die( "fscanf on /proc/meminfo in mem_mon failed" );
        }
        else
        {
            die_sans_errno(
                "matching error for fscanf on /proc/meminfo in mem_mon" );
        }
    }

    // close proc file
    if( fclose( procmem ) != 0 )
    {
        die( "couldn't close /proc/meminfo in mem_mon" );
    }

    // calculate percent mem usage
    int i;
    unsigned long int total_free_memory = 0;
    for( i = MEMFREE; i < MEM_STAT_LEN; i++ )
    {
        total_free_memory += mem_stats[i];
    }

    double percent_mem
        = ( ( double ) ( mem_stats[MEMTOTAL] - total_free_memory )
        / ( double ) mem_stats[MEMTOTAL] )
        * 100;
    
    char out_buffer[BUFFER_LEN];

    // write mem percent to output buffer
    int bytes_written = snprintf( out_buffer, BUFFER_LEN, "mem:%3.0f%%",
        percent_mem );
    if ( bytes_written < 0 )
    {
        die_sans_errno( "snprintf in mem_mon failed" );
    }
    
    multithread_write( out_buffer, ( int * ) thread_id );

    return NULL;
}
