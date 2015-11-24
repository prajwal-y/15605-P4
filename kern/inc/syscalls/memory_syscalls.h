/** @file memory_syscalls.h
 *
 *  @brief prototypes of functions for memory management functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __MEMORY_SYSCALLS_H
#define __MEMORY_SYSCALLS_H

int new_pages_handler();

int new_pages_handler_c(void *arg_packet);

int remove_pages_handler();

int remove_pages_handler_c(void *base_addr);

#endif  /* __MEMORY_SYSCALLS_H */
