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
#include "include/multithread.h"
#include "include/error.h"
#include "include/timeout.h"

#define BUFFER_LEN  20

#define USER            0
#define NICE            1
#define SYSTEM          2
#define IDLE            3
#define IOWAIT          4
#define IRQ             5
#define SOFTIRQ         6
#define STEAL           7
#define GUEST           8
#define GUEST_NICE      9
#define CPU_STAT_LEN    10

void *cpu_mon( void *thread_id )
{
    // open /proc/stat and read values from it
    FILE *procstat = fopen( "/proc/stat", "r" );
    if( procstat == NULL ) 
    {
        die( "couldn't open /proc/stat in cpu_mon" );
    }
    unsigned long int start_stats[CPU_STAT_LEN];
    int values_read = fscanf( procstat,
        "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        start_stats + USER,
        start_stats + NICE,
        start_stats + SYSTEM,
        start_stats + IDLE,
        start_stats + IOWAIT,
        start_stats + IRQ,
        start_stats + SOFTIRQ,
        start_stats + STEAL,
        start_stats + GUEST,
        start_stats + GUEST_NICE );
    if( values_read != CPU_STAT_LEN )
    {
        if( ferror( procstat ) )
        {
            die( "first fscanf on /proc/stat in cpu_mon failed" );
        }
        else
        {
            die_sans_errno(
                "matching error for first fscanf on /proc/stat in cpu_mon" );
        }
    }

    // take a time out
    timeout( 0, 250000000 );

    // reread /proc/stat
    rewind( procstat );
    unsigned long int end_stats[CPU_STAT_LEN];
    values_read = fscanf( procstat,
        "cpu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
        end_stats + USER,
        end_stats + NICE,
        end_stats + SYSTEM,
        end_stats + IDLE,
        end_stats + IOWAIT,
        end_stats + IRQ,
        end_stats + SOFTIRQ,
        end_stats + STEAL,
        end_stats + GUEST,
        end_stats + GUEST_NICE );
    if( values_read != CPU_STAT_LEN )
    {
        if( ferror( procstat ) )
        {
            die( "second fscanf on /proc/stat in cpu_mon failed" );
        }
        else
        {
            die_sans_errno(
                "matching error for second fscanf on /proc/stat in cpu_mon" );
        }
    }

    // close proc file
    if( fclose( procstat ) != 0 )
    {
        die( "couldn't close /proc/stat in cpu_mon" );
    }

    // calculate percent cpu load
    int i;
    unsigned long int total_starting_cpu_time = 0;
    unsigned long int total_ending_cpu_time = 0;
    for( i = 0; i < CPU_STAT_LEN; i++ )
    {
        total_starting_cpu_time += start_stats[i];
        total_ending_cpu_time += end_stats[i];
    }
    
    unsigned long int starting_cpu_work = total_starting_cpu_time
        - start_stats[IDLE]
        - start_stats[IOWAIT]
        - start_stats[STEAL];
    unsigned long int ending_cpu_work = total_ending_cpu_time
        - end_stats[IDLE]
        - end_stats[IOWAIT]
        - end_stats[STEAL];
    double percent_cpu_load
        = ( ( double ) ( ending_cpu_work - starting_cpu_work )
        / ( double ) ( total_ending_cpu_time - total_starting_cpu_time ) )
        * 100;
    
    char out_buffer[BUFFER_LEN];
    
    // write cpu percent to output buffer
    int bytes_written = snprintf( out_buffer, BUFFER_LEN, "cpu:%3.0f%%",
        percent_cpu_load );
    if ( bytes_written < 0 )
    {
        die_sans_errno( "snprintf in cpu_mon failed" );
    }
    
    multithread_write( out_buffer, ( int * ) thread_id );
    
    return NULL;
}
