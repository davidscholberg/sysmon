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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "sysmon.h"
#include "error.h"

#define OUT_BUF_SIZE    500

static void *separator_write( void * );

struct separator_args_t
{
    char *separators;
    int *separators_order;
    int separator_count;
};

int current_outputer = 0;
pthread_mutex_t current_outputer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t *conditions = NULL;

char out_buffer[OUT_BUF_SIZE];
int remaining_bytes = OUT_BUF_SIZE;
char *current_buffer = out_buffer;

void multithread_init( monitor_t * monitors, int *monitors_order,
    int monitor_count, char *separators, int *separators_order,
    int separator_count)
{
    int total_outputers = monitor_count + separator_count;
    
    pthread_t *monitor_threads = malloc( sizeof( pthread_t ) * monitor_count );
    if( monitor_threads == NULL )
    {
        die_sans_errno( "malloc in multithread_init failed" );
    }
    
    conditions = malloc( sizeof( pthread_cond_t ) * total_outputers );
    if( conditions == NULL )
    {
        die_sans_errno( "malloc in multithread_init failed" );
    }
    
    // initialize conditions
    int i;
    int ret = 0;
    for( i = 0; i < total_outputers; i++ )
    {
        ret = pthread_cond_init( conditions + i, NULL );
        if( ret != 0 )
        {
            die_errnum( "pthread_cond_init in multithread_init failed", ret );
        }
    }
    
    // start monitor threads
    for( i = 0; i < monitor_count; i++ )
    {
        ret = pthread_create( monitor_threads + i, NULL, monitors[i],
            monitors_order + i );
        if( ret != 0 )
        {
            die_errnum( "pthread_create in multithread_init failed", ret );
        }
    }
    
    // start separators thread
    struct separator_args_t separator_args = 
    {
        separators,
        separators_order,
        separator_count
    };
    pthread_t *separator_thread = malloc( sizeof( pthread_t ) );
    if( separator_thread == NULL )
    {
        die_sans_errno( "malloc in multithread_init failed" );
    }
    ret = pthread_create( separator_thread, NULL, separator_write,
        &separator_args );
    if( ret != 0 )
    {
        die_errnum( "pthread_create in multithread_init failed", ret );
    }

    // wait for threads to complete
    for( i = 0; i < monitor_count; i++ )
    {
        ret = pthread_join( monitor_threads[i], NULL );
        if( ret != 0 )
        {
            die_errnum( "pthread_join in multithread_init failed", ret );
        }
    }
    ret = pthread_join( *separator_thread, NULL );
    if( ret != 0 )
    {
        die_errnum( "pthread_join in multithread_init failed", ret );
    }
    
    
    // destroy conditions
    for( i = 0; i < total_outputers; i++ )
    {
        ret = pthread_cond_destroy( conditions + i );
        if( ret != 0 )
        {
            die_errnum( "pthread_cond_destroy in multithread_init failed",
                ret );
        }
    }
    
    // free memory
    free( conditions );
    free( monitor_threads );
    free( separator_thread );
    
    // write buffer to stdout
    if( printf( "%s\n", out_buffer ) < 0 )
    {
        die_sans_errno( "printf in multithread_init failed" );
    }
    
    return;
}

void multithread_write( char *str, int *thread_id )
{
    int ret = pthread_mutex_lock( &current_outputer_mutex );
    if( ret != 0 )
    {
        die_errnum( "pthread_mutex_lock in multithread_write failed", ret );
    }
    
    if( *thread_id != current_outputer )
    {
        ret = pthread_cond_wait( conditions + *thread_id - 1,
            &current_outputer_mutex );
        if( ret != 0 )
        {
            die_errnum( "pthread_cond_wait in multithread_write failed", ret );
        }
    }
    
    if( remaining_bytes > 0 )
    {
        int bytes_written = snprintf( current_buffer, remaining_bytes, "%s",
            str );
        if ( bytes_written < 0 )
        {
            die_sans_errno( "snprintf in multithread_write failed" );
        }
        
        remaining_bytes -= bytes_written;
        current_buffer += bytes_written;
    }
    
    current_outputer++;
    ret = pthread_cond_signal( conditions + *thread_id );
    if( ret != 0 )
    {
        die_errnum( "pthread_cond_signal in multithread_write failed", ret );
    }
    
    ret = pthread_mutex_unlock( &current_outputer_mutex );
    if( ret != 0 )
    {
        die_errnum( "pthread_mutex_unlock in multithread_write failed", ret );
    }
    
    return;
}

static void *separator_write( void *separator_args_arg )
{
    struct separator_args_t *separator_args = separator_args_arg;
    
    int order_index;
    int ret;
    int i;
    for( i = 0; i < separator_args->separator_count; i++ )
    {
        ret = pthread_mutex_lock( &current_outputer_mutex );
        if( ret != 0 )
        {
            die_errnum( "pthread_mutex_lock in separator_write failed", ret );
        }
        
        order_index = separator_args->separators_order[i];
        if( order_index != current_outputer )
        {
            ret = pthread_cond_wait( conditions + order_index - 1,
                &current_outputer_mutex );
            if( ret != 0 )
            {
                die_errnum( "pthread_cond_wait in separator_write failed",
                    ret );
            }
        }
        
        if( remaining_bytes > 1 )
        {
            *current_buffer = separator_args->separators[i];
            *( current_buffer + 1 ) = '\0';
            
            remaining_bytes -= 1;
            current_buffer += 1;
        }
        
        current_outputer++;
        ret = pthread_cond_signal( conditions + order_index );
        if( ret != 0 )
        {
            die_errnum( "pthread_cond_signal in multithread_write failed", ret );
        }
        
        ret = pthread_mutex_unlock( &current_outputer_mutex );
        if( ret != 0 )
        {
            die_errnum( "pthread_mutex_unlock in multithread_write failed", ret );
        }
    }
    
    return NULL;
}
