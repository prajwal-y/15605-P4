/** @file device_handler.c
 *  @brief This file contains the handlers for various
 *  	devices.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <drivers/keyboard/keyboard.h>
#include <common/errors.h>
#include <stdio.h>
#include <string.h>

/** @brief Handler for keyboard
 *
 *  This function is the handler for keyboard interrupts.
 *
 *  @return void
 */
void keyboard_device_handler_c() {
	enqueue_scancode(); /* Right now, it's the kernel driver */	
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
	
}

/** @brief Handler for COM1
 *
 *  This function is the handler for COM1 interrupts.
 *
 *  @return void
 */
void com1_device_handler_c() {
	
}

/** @brief Handler for COM2
 *
 *  This function is the handler for COM2 interrupts.
 *
 *  @return void
 */
void com2_device_handler_c() {
	
}
