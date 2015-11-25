/** @file device_handlers_asm.S
 *
 *  asm file for device handlers
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

.globl keyboard_device_handler
keyboard_device_handler:
	pusha							/* Save the general purpose registers */
	call keyboard_device_handler_c	/* Call the C handler for keyboard */
	popa							/* Restore the registers */
	iret

.globl mouse_device_handler
mouse_device_handler:
	pusha						/* Save the general purpose registers */
	call mouse_device_handler_c	/* Call the C handler for mouse */
	popa						/* Restore the registers */
	iret

.globl console_device_handler
console_device_handler:
	pusha							/* Save the general purpose registers */
	call console_device_handler_c	/* Call the C handler for console */
	popa							/* Restore the registers */
	iret

.globl com1_device_handler
com1_device_handler:
	pusha						/* Save the general purpose registers */
	call com1_device_handler_c	/* Call the C handler for COM1 */
	popa						/* Restore the registers */
	iret

.globl com2_device_handler
com2_device_handler:
	pusha						/* Save the general purpose registers */
	call com2_device_handler_c	/* Call the C handler for COM2 */
	popa						/* Restore the registers */
	iret
