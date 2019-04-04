/*
 * libTiming.h
 *
 *  Created on: March 15 , 2019
 *      Author: william.montgomery
 */

#ifndef TOOLS_TIMING_H_
#define TOOLS_TIMING_H_


#undef PERFORM_TIMINGS
#define PERFORM_TIMINGS 1

void timing_push (char  *_sourceFilename,
                  int    _lineNo,
                  char  *_tag);
void timing_free (void *args);

#ifdef PERFORM_TIMINGS
#define TIMING_TAG_PUSH(_tag) timing_push(__FILE__, __LINE__, (_tag))
#define TIMING_PUSH timing_push( __FILE__, __LINE__, 0)
#define TIMING_FREE timing_free(0);
#else
#define TIMING_TAG_PUSH(_tag)
#define TIMING_PUSH
#define TIMING_FREE
#endif
#endif /* TOOLS_TIMING_H_ */
