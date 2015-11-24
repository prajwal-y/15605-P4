/** @file thread_syscalls.h
 *
 *  @brief prototypes of functions for thread management
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __THREAD_SYSCALLS_H
#define __THREAD_SYSCALLS_H

int gettid_handler();

int gettid_handler_c();

int yield_handler();

int yield_handler_c(int tid);

int sleep_handler();

int sleep_handler_c(int ticks);

int deschedule_handler();

int deschedule_handler_c(int *reject);

int make_runnable_handler();

int make_runnable_handler_c(int tid);

unsigned int get_ticks_handler();

unsigned int get_ticks_handler_c();

int swexn_handler();

int swexn_handler_c(void *arg_packet);

#endif  /* __THREAD_SYSCALLS_H */
