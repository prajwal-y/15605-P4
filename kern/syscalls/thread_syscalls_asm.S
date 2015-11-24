/** @file thread_syscalls.S
 *
 *  Implementations of thread interrupt handlers
 *  
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>

.globl gettid_handler
gettid_handler:
	SAVE_REGS
    call gettid_handler_c
	RESTORE_REGS
	iret

.globl yield_handler
yield_handler:
	SAVE_REGS
    call yield_handler_c
	RESTORE_REGS
    iret

.globl sleep_handler
sleep_handler:
	SAVE_REGS
    call sleep_handler_c
	RESTORE_REGS
	iret

.globl deschedule_handler
deschedule_handler:
	SAVE_REGS
    call deschedule_handler_c
	RESTORE_REGS
	iret

.globl make_runnable_handler
make_runnable_handler:
	SAVE_REGS
    call make_runnable_handler_c
	RESTORE_REGS
	iret

.globl get_ticks_handler
get_ticks_handler:
	SAVE_REGS
    call get_ticks_handler_c
	RESTORE_REGS
	iret

.globl swexn_handler
swexn_handler:
	SAVE_REGS
    call swexn_handler_c
	RESTORE_REGS
	iret
