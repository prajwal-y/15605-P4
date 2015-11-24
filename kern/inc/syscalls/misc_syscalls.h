/** @file misc_syscalls.h
 *
 *  @brief prototypes o miscellaneous function calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __MISC_SYSCALLS_H
#define __MISC_SYSCALLS_H

void halt_handler();

int readfile_handler();

int readfile_handler_c(void *arg_packet);

#endif  /* __MISC_SYSCALLS_H */
