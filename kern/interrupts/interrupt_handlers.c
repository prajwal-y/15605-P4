/** @file interrupt_handlers.c
 *  @brief This file contains interrupt initialization
 *         and activation code and other interrupt helper
 *         routines
 *
 *  We initialize the timer, keyboard and the other fault 
 *  handlers in this file
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <asm.h>
#include <interrupts/ns16550.h>
#include <udriv_kern.h>
#include <udriv_registry.h>
#include <asm/asm.h>
#include <core/scheduler.h>
#include <interrupts/idt_entry.h>
#include <interrupts/interrupt_handlers.h>
#include <interrupts/device_handlers.h>
#include <interrupts/fault_handlers.h>
#include <interrupts/fault_handlers_asm.h>
#include <interrupt_defines.h>
#include <drivers/timer/timer.h>
#include <drivers/keyboard/keyboard.h>
#include <simics.h>
#include <idt.h>
#include <cr.h>
#include <syscall.h>
#include <syscalls/syscall_util.h>
#include <string.h>

static void init_serial_devices(int base_addr);
static int install_device_handlers();

/*All the interrupts initialization*/
static int install_divide_error_handler();
static int install_debug_exception_handler();
static int install_non_maskable_interrupt_handler();
static int install_breakpoint_handler();
static int install_overflow_handler();
static int install_bound_range_handler();
static int install_undefined_opcode_handler();
static int install_no_math_coprocessor_handler();
static int install_cso_handler();
static int install_invalid_tss_handler();
static int install_snp_handler();
static int install_gpf_handler();
static int install_page_fault_handler();
static int install_math_fault_handler();
static int install_alignment_check_handler();
static int install_machine_check_handler();
static int install_floating_point_exp_handler();

/** @brief The driver-library initialization function
 *
 *   Installs the timer and keyboard interrupt handler.
 *   NOTE: handler_install should ONLY install and activate the
 *   handlers; any application-specific initialization should
 *   take place elsewhere.
 *
 *   @param tickback Pointer to clock-tick callback function
 *   
 *   @return int 0 on success. negative number on failure
 **/
int install_handlers() {
	int retval = 0;
    if((retval = initialize_timer(tickback)) < 0) {
		return retval;
	}

	/* Install IDT entries for all devices */
	if((retval = install_device_handlers()) < 0) {
		return retval;
	}

	if((retval = install_divide_error_handler()) < 0) {
		return retval;
	}
	if((retval = install_debug_exception_handler()) < 0) {
		return retval;
	}
	if((retval = install_non_maskable_interrupt_handler()) < 0) {
		return retval;
	}
	if((retval = install_breakpoint_handler()) < 0) {
		return retval;
	}
	if((retval = install_overflow_handler()) < 0) {
		return retval;
	}
	if((retval = install_bound_range_handler()) < 0) {
		return retval;
	}
	if((retval = install_undefined_opcode_handler()) < 0) {
		return retval;
	}
	if((retval = install_no_math_coprocessor_handler()) < 0) {
		return retval;
	}
	if((retval = install_cso_handler()) < 0) {
		return retval;
	}
	if((retval = install_invalid_tss_handler()) < 0) {
		return retval;
	}
	if((retval = install_snp_handler()) < 0) {
		return retval;
	}
	if((retval = install_gpf_handler()) < 0) {
		return retval;
	}
	if((retval = install_page_fault_handler()) < 0) {
		return retval;
	}
	if((retval = install_math_fault_handler()) < 0) {
		return retval;
	}
	if((retval = install_alignment_check_handler()) < 0) {
		return retval;
	}
	if((retval = install_machine_check_handler()) < 0) {
		return retval;
	}
	if((retval = install_floating_point_exp_handler()) < 0) {
		return retval;
	}
	return retval;
}

/** @brief This function installs handlers for all the devices present
 *  in udriv_table
 *
 *  @return int return value of add_idt_entry
 */
int install_device_handlers() {
	int i;
	int retval = 0;
	for(i = 0; i < device_table_entries; i++) {
		dev_spec_t device = device_table[i];
		switch(device.id) {
			case UDR_KEYBOARD:
				retval = install_keyboard_handler();
				if(retval < 0) {
					return retval;
				}
				break;
			case UDR_MOUSE:				
				retval = add_idt_entry(mouse_device_handler, device.idt_slot,
										TRAP_GATE, KERNEL_DPL);
				if(retval < 0) {
					return retval;
				}
				break;
			case UDR_CONSOLE:
				retval = add_idt_entry(console_device_handler, device.idt_slot,
										INTERRUPT_GATE, KERNEL_DPL);
				if(retval < 0) {
					return retval;
				}
				break;
			case UDR_DEV_COM1:
				retval = add_idt_entry(com1_device_handler, device.idt_slot,
										INTERRUPT_GATE, KERNEL_DPL);
				if(retval < 0) {
					return retval;
				}
				init_serial_devices(COM1_IO_BASE);
				break;
			case UDR_DEV_COM2:
				retval = add_idt_entry(com2_device_handler, device.idt_slot,
										INTERRUPT_GATE, KERNEL_DPL);
				if(retval < 0) {
					return retval;
				}
				init_serial_devices(COM2_IO_BASE);
				break;
			default:
				break;
		}
	}
	return retval;
}


/** @brief Function to initialize the COM1 and COM2 serial devices
 *
 *  @param base_addr The address of the base of the serial device 
 *
 *  @return void
 */
void init_serial_devices(int base_addr) {

	/* Set DLAB to 1 to set baud rate */
	outb(base_addr + REG_LINE_CNTL, LCR_DLAB);

	/* Set LSB and MSB of the baud divider */
	outb(base_addr + REG_BAUD_LSB, 1);
	outb(base_addr + REG_BAUD_MSB, 0);

	/* Set the line control register to use 8N1 line configuration */
	outb(base_addr + REG_LINE_CNTL, CONF_8N1);
	
	/* Enable interrupt delivery and set auxiliary output in modem 
 		control register */
	outb(base_addr + REG_INT_EN, IER_TX_EMPTY_INT_EN | IER_RX_FULL_INT_EN);

	/* Auxiliary output 2 to be set in the model register */
	outb(base_addr + REG_MOD_CNTL, IER_MODEM_STATUS_INT_EN); 

}

/** @brief this function installs a handler for divide by zero fault conditions
 *
 *  @return int return value of add_idt_entry
 */
int install_divide_error_handler() {
    return add_idt_entry(divide_error_handler, IDT_DE, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for page fault
 *
 * @return int return value of add_idt_entry
 */
int install_page_fault_handler() {
	return add_idt_entry(page_fault_handler, IDT_PF, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for debug exception
 *
 * @return int return value of add_idt_entry
 */
int install_debug_exception_handler() {
	return add_idt_entry(debug_exception_handler, IDT_DB, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for debug exception
 *
 * @return int return value of add_idt_entry
 */
int install_non_maskable_interrupt_handler() {
	return add_idt_entry(non_maskable_interrupt_handler, IDT_NMI, 
							TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for breakpoint handler
 *
 * @return int return value of add_idt_entry
 */
int install_breakpoint_handler() {
	return add_idt_entry(breakpoint_handler, IDT_BP, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for overflow handler
 *
 * @return int return value of add_idt_entry
 */
int install_overflow_handler() {
	return add_idt_entry(overflow_handler, IDT_OF, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for bound range exceeded
 * fault
 *
 * @return int return value of add_idt_entry
 */
int install_bound_range_handler() {
	return add_idt_entry(bound_range_handler, IDT_BR, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for undefined opcode fault
 *
 * @return int return value of add_idt_entry
 */
int install_undefined_opcode_handler() {
	return add_idt_entry(undefined_opcode_handler, IDT_OF, 
							TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for no math coprocessor error
 *
 * @return int return value of add_idt_entry
 */
int install_no_math_coprocessor_handler() {
	return add_idt_entry(no_math_coprocessor_handler, IDT_NM, 
					TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for overflow handler
 *
 * @return int return value of add_idt_entry
 */
int install_cso_handler() {
	return add_idt_entry(cso_handler, IDT_CSO, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for invalid task segment
 * 		selector fault
 *
 * @return int return value of add_idt_entry
 */
int install_invalid_tss_handler() {
	return add_idt_entry(invalid_tss_handler, IDT_TS, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for segment not present
 * 		fault
 *
 * @return int return value of add_idt_entry
 */
int install_snp_handler() {
	return add_idt_entry(snp_handler, IDT_NP, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler stack segment fault
 *
 * @return int return value of add_idt_entry
 */
int install_ssf_handler() {
	return add_idt_entry(ssf_handler, IDT_SS, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for general protection
 * 			fault
 *
 * @return int return value of add_idt_entry
 */
int install_gpf_handler() {
	return add_idt_entry(gpf_handler, IDT_GP, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for math fault
 *
 * @return int return value of add_idt_entry
 */
int install_math_fault_handler() {
	return add_idt_entry(math_fault_handler, IDT_MF, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for alignment check
 *
 * @return int return value of add_idt_entry
 */
int install_alignment_check_handler() {
	return add_idt_entry(alignment_check_handler, IDT_AC, 
							TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for machine check exception
 *
 * @return int return value of add_idt_entry
 */
int install_machine_check_handler() {
	return add_idt_entry(machine_check_handler, IDT_MC, TRAP_GATE, KERNEL_DPL);
}

/** @brief This function installs a handler for floating point exception
 *
 * @return int return value of add_idt_entry
 */
int install_floating_point_exp_handler() {
	return add_idt_entry(floating_point_exp_handler, IDT_XF, 
							TRAP_GATE, KERNEL_DPL);
}

/** @brief this function acknowledges the interrupt
 *
 *  We send an ACK to the PIC control port to indicate
 *  that we have finished processing the interrupt
 *
 *  @return void
 */
void acknowledge_interrupt() {
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
