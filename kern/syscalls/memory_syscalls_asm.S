/** @file memory_syscalls_asm.S
 *  
 *  handlers for memory management system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>

.globl new_pages_handler
new_pages_handler:
	SAVE_REGS
    call new_pages_handler_c
	RESTORE_REGS
    iret

.globl remove_pages_handler
remove_pages_handler:
	SAVE_REGS
    call remove_pages_handler_c
	RESTORE_REGS
    iret
