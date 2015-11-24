/** @file lifecycle_syscalls_asm.S
 *  
 *  handlers for lifecycle management system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>
#include <simics.h>

.globl fork_handler
fork_handler:
	SAVE_REGS			/* Using SAVE_REGS instead of PUSHA */
    call fork_handler_c	/* Call fork handler C function */
	RESTORE_REGS		/* Restore all registers except %EAX */
	iret

.globl thread_fork_handler
thread_fork_handler:
	SAVE_REGS					/* Using SAVE_REGS instead of PUSHA */
    call thread_fork_handler_c	/* Call thread fork handler C function */
	RESTORE_REGS				/* Restore all registers except %EAX */
	iret

.globl exec_handler
exec_handler:
	SAVE_REGS				/* Using SAVE_REGS instead of PUSHA */
    call exec_handler_c		/* Call exec_handler C function */
	RESTORE_REGS			/* Restore all register except EAX */
    iret

.globl set_status_handler
set_status_handler:
    pusha
    pushl %esi
    call set_status_handler_c
    popl %esi
    popa
    iret

.globl vanish_handler
vanish_handler:
    pusha
    pushl %esi
    call vanish_handler_c
    popl %esi
    popa
    iret

.globl wait_handler
wait_handler:
	SAVE_REGS
    call wait_handler_c
	RESTORE_REGS
    iret
