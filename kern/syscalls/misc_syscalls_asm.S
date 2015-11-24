/** @file misc_syscalls_asm.S
 *  
 *  handlers for miscellaneous system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>

.globl halt_handler
halt_handler:
    call sim_halt
    cli             /* In case sim_halt is a no-op */
    hlt             /* No more interrupts, no more instructions */

.globl readfile_handler
readfile_handler:
	SAVE_REGS
    call readfile_handler_c
	RESTORE_REGS
    iret
