/** @file syscall_handlers.c
 *  @brief This file contains functions to install
 *         various syscall handlers.
 *  
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <asm.h>
#include <syscalls/syscall_handlers.h>
#include <syscall_int.h>
#include <interrupts/idt_entry.h>
#include <idt.h>
#include <syscalls/thread_syscalls.h>
#include <syscalls/console_syscalls.h>
#include <syscalls/lifecycle_syscalls.h>
#include <syscalls/misc_syscalls.h>
#include <syscalls/memory_syscalls.h>
#include <syscalls/system_check_syscalls.h>

static int install_print_handler();
static int install_fork_handler();
static int install_thread_fork_handler();
static int install_exec_handler();
static int install_set_status_handler();
static int install_halt_handler();
static int install_wait_handler();
static int install_vanish_handler();
static int install_new_pages_handler();
static int install_remove_pages_handler();
static int install_readline_handler();
static int install_gettid_handler();
static int install_yield_handler();
static int install_deschedule_handler();
static int install_make_runnable_handler();
static int install_get_ticks_handler();
static int install_sleep_handler();
static int install_swexn_handler();
static int install_readfile_handler();
static int install_set_term_color_handler();
static int install_set_cursor_pos_handler();
static int install_get_cursor_pos_handler();
static int install_getchar_handler();
static int install_memcheck_handler();

/** @brief The syscall handlers initialization function
 *
 *   @return int 0 on success and negative number on failure
 **/
int install_syscall_handlers() {
	int retval = 0;
    if((retval = install_print_handler()) < 0) {
		return retval;
	}
	if((retval = install_fork_handler()) < 0) {
		return retval;
	}
	if((retval = install_thread_fork_handler()) < 0) {
		return retval;
	}
	if((retval = install_exec_handler()) < 0) {
		return retval;
	}
	if((retval = install_set_status_handler()) < 0) {
		return retval;
	}
	if((retval = install_halt_handler()) < 0) {
		return retval;
	}
    if((retval = install_wait_handler()) < 0) {
		return retval;
	}
    if((retval = install_vanish_handler()) < 0) {
		return retval;
	}
    if((retval = install_new_pages_handler()) < 0) {
		return retval;
	}
    if((retval = install_remove_pages_handler()) < 0) {
		return retval;
	}
    if((retval = install_readline_handler()) < 0) {
		return retval;
	}
    if((retval = install_yield_handler()) < 0) {
		return retval;
	}
    if((retval = install_memcheck_handler()) < 0) {
		return retval;
	}
    if((retval = install_gettid_handler()) < 0) {
		return retval;
	}
    if((retval = install_deschedule_handler()) < 0) {
		return retval;
	}
    if((retval = install_make_runnable_handler()) < 0) {
		return retval;
	}
    if((retval = install_get_ticks_handler()) < 0) {
		return retval;
	}
    if((retval = install_sleep_handler()) < 0) {
		return retval;
	}
    if((retval = install_swexn_handler()) < 0) {
		return retval;
	}
    if((retval = install_readfile_handler()) < 0) {
		return retval;
	}
    if((retval = install_set_term_color_handler()) < 0) {
		return retval;
	}
    if((retval = install_set_cursor_pos_handler()) < 0) {
		return retval;
	}
    if((retval = install_get_cursor_pos_handler()) < 0) {
		return retval;
	}
    if((retval = install_getchar_handler()) < 0) {
		return retval;
	}
    return retval;
}

/** @brief this function installs a handler for gettid
 *
 *  @return int return value of add_idt_entry
 */
int install_gettid_handler() {
    return add_idt_entry(gettid_handler, GETTID_INT, TRAP_GATE, USER_DPL);
}

/** @brief this function installs a handler for print
 *
 *  @return int return value of add_idt_entry
 */
int install_print_handler() {
    return add_idt_entry(print_handler, PRINT_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for fork
 *
 *  @return int return value of add_idt_entry
 */
int install_fork_handler() {
	return add_idt_entry(fork_handler, FORK_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for thread fork
 *
 *  @return int return value of add_idt_entry
 */
int install_thread_fork_handler() {
	return add_idt_entry(thread_fork_handler, THREAD_FORK_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for exec
 *
 *  @return int return value of add_idt_entry
 */
int install_exec_handler() {
	return add_idt_entry(exec_handler, EXEC_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for set_status
 *
 *  @return int return value of add_idt_entry
 */
int install_set_status_handler() {
	return add_idt_entry(set_status_handler, SET_STATUS_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for halt
 *
 *  @return int return value of add_idt_entry
 */
int install_halt_handler() {
	return add_idt_entry(halt_handler, HALT_INT, TRAP_GATE, USER_DPL);
}
/** @brief Function to install a handler for wait syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_wait_handler() {
	return add_idt_entry(wait_handler, WAIT_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for vanish syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_vanish_handler() {
	return add_idt_entry(vanish_handler, VANISH_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for new_pages syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_new_pages_handler() {
	return add_idt_entry(new_pages_handler, NEW_PAGES_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for readline syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_readline_handler() {
	return add_idt_entry(readline_handler, READLINE_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for remove_pages syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_remove_pages_handler() {
	return add_idt_entry(remove_pages_handler, REMOVE_PAGES_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for yield syscall
 *
 *  @return void
 */
int install_yield_handler() {
	return add_idt_entry(yield_handler, YIELD_INT, TRAP_GATE, USER_DPL);
}

int install_memcheck_handler() {
    return add_idt_entry(memory_check_handler, SYSCALL_RESERVED_1, 
							INTERRUPT_GATE, USER_DPL);
}

/** @brief Function to install a handler for sleep syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_sleep_handler() {
	return add_idt_entry(sleep_handler, SLEEP_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for deschedule syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_deschedule_handler() {
	return add_idt_entry(deschedule_handler, DESCHEDULE_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for make_runnable syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_make_runnable_handler() {
	return add_idt_entry(make_runnable_handler, MAKE_RUNNABLE_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for get_ticks syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_get_ticks_handler() {
	return add_idt_entry(get_ticks_handler, GET_TICKS_INT, 
							TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for swexn handler syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_swexn_handler() {
	return add_idt_entry(swexn_handler, SWEXN_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for readfile syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_readfile_handler() {
	return add_idt_entry(readfile_handler, READFILE_INT, TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for set_term_color syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_set_term_color_handler() {
	return add_idt_entry(set_term_color_handler, SET_TERM_COLOR_INT, 
                         TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for set_cursor_pos syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_set_cursor_pos_handler() {
	return add_idt_entry(set_cursor_pos_handler, SET_CURSOR_POS_INT, 
                         TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for get_cursor_pos syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_get_cursor_pos_handler() {
	return add_idt_entry(get_cursor_pos_handler, GET_CURSOR_POS_INT, 
                         TRAP_GATE, USER_DPL);
}

/** @brief Function to install a handler for getchar syscall
 *
 *  @return int return value of add_idt_entry
 */
int install_getchar_handler() {
	return add_idt_entry(getchar_handler, GETCHAR_INT, 
                         TRAP_GATE, USER_DPL);
}
