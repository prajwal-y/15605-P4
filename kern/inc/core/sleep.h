/** @file sleep.h
 *
 *  Header file for sleep.c
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef __SLEEP_H
#define __SLEEP_H
#include <core/thread.h>

int do_sleep();

thread_struct_t *get_sleeping_thread();

void init_sleeping_threads();

#endif  /* __SLEEP_H */
