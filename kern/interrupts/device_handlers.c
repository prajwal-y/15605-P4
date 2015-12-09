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

#define DEV_RCV_INT 4

/** @brief Handler for keyboard
 *
 *  This function is the handler for keyboard interrupts.
 *
 *  @return void
 */
void keyboard_device_handler_c() {
	message_t msg = inb(KEYBOARD_PORT);
	udriv_send_interrupt(UDR_KEYBOARD, msg, 1);
	acknowledge_interrupt();
}

/** @brief Handler for mouse
 *
 *  This function is the handler for mouse interrupts.
 *
 *  @return void
 */
void mouse_device_handler_c() {
    //not implemented
}

/** @brief Handler for console device
 *
 *  This function is the handler for console interrupts.
 *
 *  @return void
 */
void console_device_handler_c() {
    //not implemented
}

/** @brief Handler for COM1
 *
 *  This function is the handler for COM1 interrupts.
 *
 *  @return void
 */
void com1_device_handler_c() {
    udriv_struct_t *driv = get_udriv_from_id(UDR_DEV_COM1);
    if (driv != NULL) {
        if (driv->in_bytes == 1) {
		    message_t msg = inb(driv->in_port);
		    udriv_send_interrupt(UDR_DEV_COM1, msg, 1);
        }
        else {
			udriv_send_interrupt(UDR_DEV_COM1, (message_t)0, 0);
        }
    }
	else {
		inb(COM1_IO_BASE + REG_INT_ID);
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
    udriv_struct_t *driv = get_udriv_from_id(UDR_DEV_COM2);
    if (driv != NULL) {
        if (driv->in_bytes == 1) {
		    message_t msg = inb(driv->in_port);
		    udriv_send_interrupt(UDR_DEV_COM2, msg, 1);
        }
        else {
			udriv_send_interrupt(UDR_DEV_COM2, (message_t)0, 0);
        }
    }
	acknowledge_interrupt();
}
