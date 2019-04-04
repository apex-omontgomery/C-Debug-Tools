/* -------------------------------------------------------------------------- */
/* FILE: libTiming.c                                                    */
/*                                                                            */
/* DESC: These procedures provide incremental timing records                  */
/*                                                                            */
/* AUTH: William Montgomery  March 15 2019                                    */
/*                                                                            */
/* -------------------------------------------------------------------------- */


#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#include "timingUtilities.h"

#define MAX_TIMINGS 1000
#define MAX_TAG_SIZE 20


struct _timing_item
{

    char           _tag[MAX_TAG_SIZE];
    const char    *_file;
    int            _line;
    struct timeval _time;
};

struct _timing_collection
{

    struct _timing_item collection[MAX_TIMINGS];
    int count;
};

struct _timing_config
{

    pthread_key_t *timingKey;
    struct _timing_collection *collection;
    int  isEnabled;
};

struct _timing_config _timing =
{
    .timingKey = 0,
    .collection = 0,
    .isEnabled = PERFORM_TIMINGS || 0
};

// Prevent duplicate calling of the key creation.
pthread_once_t init_done = PTHREAD_ONCE_INIT;

void timing_pthread_key_create_destuctor (void);
int timing_pthread_getspecific (struct _timing_collection **_collection);
int timing_pthread_setspecific (pthread_key_t *key,
                                struct _timing_collection *newCollection);
void timing_insert (struct _timing_collection *_timing,
                    char  *_sourceFilename,
                    int    _lineNo,
                    char  *_tag);


int timing_pthread_getspecific (struct _timing_collection **_collection)
{
    int error = 0;
    errno = 0;
    if (!_timing.timingKey)
    {
        error =  pthread_once (&init_done, timing_pthread_key_create_destuctor);
        if (error)
        {
            return error;
        }
    }
    if (!_timing.collection)
    {
        struct _timing_collection *newCollection = calloc (1, sizeof (newCollection));
        newCollection->count = 0;
        error = timing_pthread_setspecific (_timing.timingKey, newCollection);
        if (error)
        {
            return error;
        }
    }
    if (_timing.timingKey)
    {
        *_collection = ((struct _timing_collection *) pthread_getspecific (*_timing.timingKey));
        if (!_collection)
        {
            error = 1;
        }
    }
    else
    {
        error = 1;
    }
    return error;
}

int timing_pthread_setspecific (pthread_key_t *key, struct _timing_collection *newCollection)
{
    int error = 0;
    if (key)
    {
        pthread_setspecific (*key, newCollection);
        error = errno;
        if (error)
        {
            printf ("Error when attempting to setspecific: %s\n", strerror (error));
        }
    }
    return error;
}


void timing_pthread_key_create_destuctor (void)
{
    int error = pthread_key_create (_timing.timingKey, timing_free);
    if (error)
    {
        printf ("Cannot create the key with destructor: %s\n", strerror (error));
    }
}


void timing_insert (struct _timing_collection *_timing,
                    char  *_sourceFilename,
                    int    _lineNo,
                    char  *_tag)
{
#ifdef PERFORM_TIMINGS
    if (_timing->count >= MAX_TIMINGS)
    {
        return;
    }
    struct timeval *_new_time = & (_timing->collection[_timing->count]._time);
    if (gettimeofday (_new_time, 0) == -1)
    {
        return;
        // some time error occurred
        // don't add to list.
    }
    _timing->collection[_timing->count]._file = _sourceFilename;
    _timing->collection[_timing->count]._line = _lineNo;
    if (_tag != 0)
    {
        size_t _len = strlen (_tag);
        if (strlen (_tag) >= MAX_TAG_SIZE)
        {
            strncpy (_timing->collection[_timing->count]._tag, _tag, MAX_TAG_SIZE - 1);
        }
        else
        {
            strncpy (_timing->collection[_timing->count]._tag, _tag, _len);
        }
    }
    _timing->count++;

#endif
}


void timing_push (char  *_sourceFilename,
                  int    _lineNo,
                  char  *_tag)
{
#ifdef PERFORM_TIMINGS
    struct _timing_collection *_collection;
    int error =  timing_pthread_getspecific (&_collection);
    if (!error)
    {
        timing_insert (_collection, _sourceFilename, _lineNo, _tag);
    }
#endif
}


void timing_free (void *args)
{
#ifdef PERFORM_TIMINGS
    struct _timing_collection *_collection;
    int error =  timing_pthread_getspecific (&_collection);

    if (!error && _collection)
    {
        free (_collection);
        _collection = 0;
    }
#endif
}

