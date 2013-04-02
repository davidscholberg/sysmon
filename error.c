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

void die( const char * msg )
{
    perror( msg );
    exit( EXIT_FAILURE );
}

void die_sans_errno( const char * msg )
{
    fprintf( stderr, "%s\n", msg );
    exit( EXIT_FAILURE );
}

void die_errnum( const char * msg, int errnum )
{
    errno = errnum;
    die( msg );
}
