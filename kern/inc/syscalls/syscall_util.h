/** @file syscall_util.h
 *  @brief utility functions for syscall handlers
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __SYSCALL_UTIL_H
#define __SYSCALL_UTIL_H

#include <ureg.h>
#include <core/thread.h>

#define ERR_CODE_AVAIL 1
#define ERR_CODE_ABSENT 0

#define EFLAGS_RESERVED 0x00000002
#define EFLAGS_IOPL 0x00003000 
#define EFLAGS_IF 0x00000200 
#define EFLAGS_ALIGNMENT_CHECK 0xFFFbFFFF

int setup_kernel_stack(ureg_t *ureg, void *kernel_stack_base);

void populate_ureg(ureg_t *ureg, int err_code_available, 
                   thread_struct_t *curr_thread);

int is_pointer_valid(void *ptr, int bytes);

int copy_user_data(char *buf, char *ptr, int max_size);

#endif  /*__SYSCALL_UTIL_H */
