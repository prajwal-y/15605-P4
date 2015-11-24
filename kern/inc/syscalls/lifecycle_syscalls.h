/** @file lifecycle_syscalls.h
 *
 *  @brief prototypes of functions for task lifecycle functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __LIFECYCLE_SYSCALLS_H
#define __LIFECYCLE_SYSCALLS_H

int fork_handler();

int fork_handler_c();

int thread_fork_handler();

int thread_fork_handler_c();

int exec_handler();

int exec_handler_c();

void set_status_handler();

void set_status_handler_c(int status);

int wait_handler(int *status_ptr);

int wait_handler_c(int *status_ptr);

int vanish_handler();

int vanish_handler_c();

#endif  /* __LIFECYCLE_SYSCALLS_H */
