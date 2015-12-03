/** @file device_handler.c
 *  @brief This file contains the handlers for various
 *  	devices.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <drivers/keyboard/keyboard.h>
#include <interrupts/interrupt_handlers.h>
#include <interrupts/ns16550.h>
#include <udriv/udriv.h>
#include <udriv_registry.h>
#include <syscall.h>
#include <common/errors.h>
#include <stdio.h>
#include <string.h>
#include <simics.h>
#include <asm.h>
#include <keyhelp.h>

/** @brief Handler for keyboard
 *
 *  This function is the handler for keyboard interrupts.
 *
 *  @return void
 */
void keyboard_device_handler_c() {
	//enqueue_scancode(); /* Right now, it's the kernel driver */	
	message_t msg = inb(KEYBOARD_PORT);
	udriv_send_interrupt(UDR_KEYBOARD, msg, 1);
	acknowledge_interrupt();
}

/** @brief Handler for mouse
 *
 *  This function is the handler for keyboard interrupts.
 *
 *  @return void
 */
void mouse_device_handler_c() {
	
}

/** @brief Handler for console
 *
 *  This function is the handler for keyboard interrupts.
 *
 *  @return void
 */
void console_device_handler_c() {	
	lprintf("Console interrupt received");
}

/** @brief Handler for COM1
 *
 *  This function is the handler for COM1 interrupts.
 *
 *  @return void
 */
void com1_device_handler_c() {
	int c = inb(COM1_IO_BASE + REG_INT_ID);
	if(c & 4) {
		message_t msg = inb(COM1_IO_BASE);
		udriv_send_interrupt(UDR_DEV_COM1, msg, 1);
	}
	acknowledge_interrupt();
}

/** @brief Handler for COM2
 *
 *  This function is the handler for COM2 interrupts.
 *
 *  @return void
 */
void com2_device_handler_c() {
	int c = inb(COM2_IO_BASE + REG_INT_ID);
	if(c & 4) {
		message_t msg = inb(COM2_IO_BASE);
		udriv_send_interrupt(UDR_DEV_COM2, msg, 1);
	}
	acknowledge_interrupt();
}
