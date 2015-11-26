/** @file udriv_syscalls_asm.S
 *
 *  Implementations of udriv system calls
 *  
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>

.globl udriv_register_handler
udriv_register_handler:
	SAVE_REGS
    call handle_udriv_register
	RESTORE_REGS
	iret

.globl udriv_deregister_handler
udriv_deregister_handler:
	SAVE_REGS
    call handle_udriv_deregister
	RESTORE_REGS
	iret

.globl udriv_send_handler
udriv_send_handler:
	SAVE_REGS
    call handle_udriv_send
	RESTORE_REGS
	iret

.globl udriv_wait_handler
udriv_wait_handler:
	SAVE_REGS
    call handle_udriv_wait
	RESTORE_REGS
	iret

.globl udriv_inb_handler
udriv_inb_handler:
	SAVE_REGS
    call handle_udriv_inb
	RESTORE_REGS
	iret

.globl udriv_outb_handler
udriv_outb_handler:
	SAVE_REGS
    call handle_udriv_outb
	RESTORE_REGS
	iret

.globl udriv_mmap_handler
udriv_mmap_handler:
	SAVE_REGS
    call handle_udriv_mmap
	RESTORE_REGS
	iret
