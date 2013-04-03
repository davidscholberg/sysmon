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

#include <stdlib.h>
#include <string.h>
#include "include/sysmon.h"
#include "include/multithread.h"
#include "include/error.h"
#include "include/cpu_mon.h"
#include "include/mem_mon.h"
#include "include/io_mon.h"
#include "include/date_mon.h"

int main (int argc, char **argv)
{
    if( argc != 2 )
    {
        die_sans_errno(
            "Usage:\n"
            "    sysmon <format_string>\n\n"
            "    sysmon is a terminal app that outputs various system\n"
            "    information on a single line to standard output. sysmon\n"
            "    exits after the line is printed.\n\n"
            "    format_string is a string of characters that represents how\n"
            "    the monitors will be displayed. Each monitor is represented\n"
            "    with a unique character, which are listed below:\n\n"
            "    c - cpu activity monitor\n"
            "    m - used memory monitor\n"
            "    i - io activity monitor\n"
            "    d - date monitor\n\n"
            "    All other characters that appear in format_string are\n"
            "    printed as is.\n\n"
            "Example:\n"
            "    sysmon \"c | m | i | d\"" );
    }
    
//    char mon_format[] = "c | m | i | d";
    char *mon_format = argv[1];
    int mon_format_size = strlen( mon_format );
    
    // pass 1, determine how many separators and monitors in mon_format
    int i;
    int monitor_count = 0;
    for( i = 0; i < mon_format_size; i++ )
    {
        switch( mon_format[i] )
        {
            case 'c':
            case 'm':
            case 'i':
            case 'd':
                monitor_count++;
        }
    }
    int separator_count = mon_format_size - monitor_count;
    
    // allocate space for separators and monitors arrays
    monitor_t *monitors = malloc( sizeof( monitor_t ) * monitor_count );
    int *monitors_order = malloc( sizeof( int ) * monitor_count );
    char *separators = malloc( sizeof( char ) * separator_count );
    int *separators_order = malloc( sizeof( int ) * separator_count );

    // pass 2, populate separators and monitors arrays
    int monitors_index = 0;
    int separators_index = 0;    
    for( i = 0; i < mon_format_size; i++ )
    {
        switch( mon_format[i] )
        {
            case 'c':
                monitors[monitors_index] = cpu_mon;
                monitors_order[monitors_index] = i;
                monitors_index++;
                break;
            case 'm':
                monitors[monitors_index] = mem_mon;
                monitors_order[monitors_index] = i;
                monitors_index++;
                break;
            case 'i':
                monitors[monitors_index] = io_mon;
                monitors_order[monitors_index] = i;
                monitors_index++;
                break;
            case 'd':
                monitors[monitors_index] = date_mon;
                monitors_order[monitors_index] = i;
                monitors_index++;
                break;
            default:
                separators[separators_index] = mon_format[i];
                separators_order[separators_index] = i;
                separators_index++;
                break;
        }
    }

    multithread_init( monitors, monitors_order, monitor_count, separators,
        separators_order, separator_count );
    
    free( monitors );
    free( monitors_order );
    free( separators );
    free( separators_order );

    return 0;
}
