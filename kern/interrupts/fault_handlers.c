/** @file fault_handlers.c
 *  @brief This file contains the handlers for various
 *  	faults.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <asm.h>
#include <asm/asm.h>
#include <core/scheduler.h>
#include <interrupt_defines.h>
#include <drivers/timer/timer.h>
#include <vm/vm.h>
#include <simics.h>
#include <console.h>
#include <common/errors.h>
#include <core/context.h>
#include <core/wait_vanish.h>
#include <stdio.h>
#include <idt.h>
#include <cr.h>
#include <syscall.h>
#include <syscalls/syscall_util.h>
#include <string.h>
#include <common/assert.h>
#include <core/thread.h>

#define THREAD_KILL_EXIT_STATUS -2
#define THREAD_KILL_MSG_LEN 256

/*Static functions*/
static void *setup_swexn_stack(void *esp3, ureg_t *ureg, void *arg);
static void update_fault_stack(void *esp3, swexn_handler_t eip, 
                               thread_struct_t *curr_thread);
static void kill_current_thread(int cause);
void handle_fault(int cause);
int invoke_swexn_handler(int cause);

/** @brief Callback function for the timer handler
 *
 *  This function invokes the context switch during every
 *  timer tick
 *
 *  @return void
 */
void tickback(unsigned int ticks) {
	context_switch();
}

/** @brief this function handles a divide by zero error condition.
 *
 *  @return void
 */
void divide_error_handler_c() {
	handle_fault(SWEXN_CAUSE_DIVIDE);
}

/** @brief This function handles the page fault
 *
 *  The page fault handler checks the address that 
 *  caused page fault. If the address is a COW page fault,
 *  then the handler invokes the COW handler using the VM
 *  module. If not, then it checks for the swexn handler
 *  installed. If the handler is not installed, then the
 *  page fault handler kills the thread.
 *
 * @return Void
 */
void page_fault_handler_c() {
	void *page_fault_addr = (void *)get_cr2();
	
	if(is_addr_cow(page_fault_addr)) {
		if(handle_cow(page_fault_addr) < 0) {
			kill_current_thread(SWEXN_CAUSE_PAGEFAULT);
		}
	} 
    else {
        handle_fault(SWEXN_CAUSE_PAGEFAULT);
    } 
}

/** @brief this function handles a debug exception
 *
 *  @return void
 */
void debug_exception_handler_c() {
	handle_fault(SWEXN_CAUSE_DEBUG);
}

/** @brief this function handles a non maskable interrupt 
 *
 *  @return void
 */
void non_maskable_interrupt_handler_c() {
    kill_current_thread(IDT_NMI);
}

/** @brief this function handles a breakpoint exception
 *
 *  @return void
 */
void breakpoint_handler_c() {
	handle_fault(SWEXN_CAUSE_BREAKPOINT);
}

/** @brief this function handles a overflow exception
 *
 *  @return void
 */
void overflow_handler_c() {
	handle_fault(SWEXN_CAUSE_OVERFLOW);
}

/** @brief this function handles a bound range exception
 *
 *  @return void
 */
void bound_range_handler_c() {
	handle_fault(SWEXN_CAUSE_BOUNDCHECK);
}

/** @brief this function handles a undefined opcode error
 *
 *  @return void
 */
void undefined_opcode_handler_c() {
	handle_fault(SWEXN_CAUSE_OPCODE);
}

/** @brief this function handles a NMC error
 *
 *  @return void
 */
void no_math_coprocessor_handler_c() {
	kill_current_thread(IDT_NM);
}

/** @brief this function handles coprocessor segment overrun
 * 	error
 *
 *  @return void
 */
void cso_handler_c() {
	kill_current_thread(IDT_CSO);
}

/** @brief this function handles a invalid TSS error
 *
 *  @return void
 */
void invalid_tss_handler_c() {
	kill_current_thread(IDT_TS);
}

/** @brief this function handles a segment not present error
 *
 *  @return void
 */
void snp_handler_c() {
	handle_fault(SWEXN_CAUSE_SEGFAULT);
}

/** @brief this function handles a SSF
 *
 *  @return void
 */
void ssf_handler_c() {
	handle_fault(SWEXN_CAUSE_STACKFAULT);
}

/** @brief this function handles a GPF
 *
 *  @return void
 */
void gpf_handler_c() {
	handle_fault(SWEXN_CAUSE_PROTFAULT);
}

/** @brief this function handles a math fault
 *
 *  @return void
 */
void math_fault_handler_c() {
	kill_current_thread(IDT_MF);
}

/** @brief this function handles an alignment fault
 *
 *  @return void
 */
void alignment_check_handler_c() {
	handle_fault(SWEXN_CAUSE_ALIGNFAULT);
}

/** @brief this function handles a MCE exception
 *
 *  @return void
 */
void machine_check_handler_c() {
	kill_current_thread(IDT_MC);
}

/** @brief this function handles a floating point exception
 *
 *  @return void
 */
void floating_point_exp_handler_c() {
	handle_fault(SWEXN_CAUSE_FPUFAULT);
}

/** @brief Function to check if a swexn handler is installed.
 * If installed, the handler is invoked. Otherwise the thread
 * is killed.
 *
 * @return void
 */
void handle_fault(int cause) {
	if (invoke_swexn_handler(cause) == 0) {
        return;
    }
    else {
        kill_current_thread(cause);
	}
}

/** @brief Function to invoke the swexn handler installed
 *	
 *	This function checks for the swexn handler installed. If
 *	a handler is installed, the setup for the invocation of
 *	the handler is done.
 *
 *	@param cause Cause of the fault to added in the ureg
 *
 *	@return 0 if successful, -1 on error
 */
int invoke_swexn_handler(int cause) {
	task_struct_t *curr_task = get_curr_task();
	thread_struct_t *curr_thread = get_curr_thread();
	if(curr_task->eip == NULL) {
		return ERR_FAILURE;
	}
	ureg_t ureg;

	ureg.cause = cause;
	ureg.cr2 = get_cr2();
	populate_ureg(&ureg, ERR_CODE_AVAIL, curr_thread);
	void *stack_bottom = setup_swexn_stack(curr_task->swexn_esp, 
											&ureg, curr_task->swexn_args);
	update_fault_stack(stack_bottom, curr_task->eip, curr_thread);
	curr_task->eip = NULL;  /* Deregister the handler */
	return 0;
}

/** @brief setup the stack for the swexn handler
 *
 *  @param esp3 the stack where the swexn handler will run
 *  @param ureg the registers for the swexn handler
 *  @param arg the arguments to the swexn handler
 *  @return void* the bottom of the stack 
 */
void *setup_swexn_stack(void *esp3, ureg_t *ureg, void *arg) {
    void *stack_bottom = (char *)esp3 - sizeof(ureg_t);
    memcpy(stack_bottom, ureg, sizeof(ureg_t));

    *((int *)(stack_bottom) - 1) = (int)stack_bottom;
    *((int *)(stack_bottom) - 2) = (int)arg;
    *((int *)(stack_bottom) - 3) = (int)ureg->eip;
    return (int *)stack_bottom - 3;
}

/** @brief update the kernel stack to change return esp and eip
 *
 *  @param esp the new esp valur to be set in the kernel stack
 *  @param eip the first instruction of the fault handler
 *  @return void
 */
void update_fault_stack(void *esp, swexn_handler_t eip, 
                        thread_struct_t *curr_thread) {
    *((int *)(curr_thread->k_stack_base) - 2) = (int)esp;
    *((int *)(curr_thread->k_stack_base) - 5) = (int)eip;
}

/** @brief Function to kill the current thread.
 *
 * 	This function prints some thread info to the console and
 * 	then kills the thread
 *
 *  @return does not return
 */
void kill_current_thread(int cause) {
	putbytes("Critical error in thread! Killing it...\n", 40);
	ureg_t ureg;
    char buf[THREAD_KILL_MSG_LEN];
    thread_struct_t *curr_thread = get_curr_thread();
    char *fmt = "Fault code: %d\n \
Thread id: %d\n \
eax = %p    ecx = %p\n \
edx = %p    ebx = %p\n \
esp = %p    ebp = %p\n \
esi = %p    edi = %p\n \
eip = %p\n";
	populate_ureg(&ureg, ERR_CODE_AVAIL, curr_thread);
    snprintf(buf, THREAD_KILL_MSG_LEN, fmt, cause, curr_thread->id,
			 ureg.eax, ureg.ecx, 
             ureg.edx, ureg.ebx, ureg.esp, ureg.ebp, ureg.esi,
             ureg.edi, ureg.eip);
    putbytes(buf, strlen(buf));
	get_curr_task()->exit_status = THREAD_KILL_EXIT_STATUS;
	do_vanish();
}
