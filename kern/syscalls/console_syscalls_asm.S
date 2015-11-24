/** @file console_syscalls.S
 *
 *  Implementations of console I/O
 *  
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <syscalls/syscall_util_asm.h>

.globl print_handler
print_handler:
	SAVE_REGS
    call print_handler_c
	RESTORE_REGS
	iret

.globl readline_handler
readline_handler:
	SAVE_REGS
    call readline_handler_c
	RESTORE_REGS
	iret

.globl set_term_color_handler
set_term_color_handler:
	SAVE_REGS
    call set_term_color_handler_c
	RESTORE_REGS
	iret

.globl set_cursor_pos_handler
set_cursor_pos_handler:
	SAVE_REGS
    call set_cursor_pos_handler_c
	RESTORE_REGS
	iret

.globl get_cursor_pos_handler
get_cursor_pos_handler:
	SAVE_REGS
    call get_cursor_pos_handler_c
	RESTORE_REGS
	iret

.globl getchar_handler
getchar_handler:
	SAVE_REGS
    call getchar_handler_c
	RESTORE_REGS
	iret
