/** @file keyboard.c
 *  @brief implementation of functions defined in keyboard.h
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadpad)
 */
#include <drivers/keyboard/keyboard.h>
#include <drivers/keyboard/keyboard_circular_buffer.h>
#include <drivers/keyboard/keyboard_handler.h>
#include <keyhelp.h>
#include <common/errors.h>
#include <interrupts/idt_entry.h>
#include <interrupts/interrupt_handlers.h>
#include <asm.h>
#include <list/list.h>
#include <stddef.h>
#include <simics.h>
#include <console.h>
#include <sync/cond_var.h>

cond_t readline_cond_var;
mutex_t readline_mutex;

/** @brief function to install the handler and initialize
 *         our scancode buffer 
 *
 *  @return int 0 on success. -ve integer on failure
 */
int install_keyboard_handler() {
	int retval = 0;
    if((retval = cond_init(&readline_cond_var)) < 0) {
		return retval;
	}
    if((retval = mutex_init(&readline_mutex)) < 0) {
		return retval;
	}
    retval = add_idt_entry(keyboard_handler, KEY_IDT_ENTRY, TRAP_GATE, KERNEL_DPL);
	return retval;
}

/** @brief add the scancode to the end of our queue and return
 *
 *  @return void 
 */
void enqueue_scancode() {
    unsigned char in = inb(KEYBOARD_PORT);

    kh_type key = process_scancode(in);
    if ((KH_HASDATA(key) != 0) && (KH_ISMAKE(key) == 0)) {
        char c = KH_GETCHAR(key);
        add_keystroke(KH_GETCHAR(key));
        if (c == '\n') {
            cond_signal(&readline_cond_var);
        }
    }
    acknowledge_interrupt();
    return;
}

/** @brief Returns the next character in the keyboard buffer
 *
 *  This function does not block if there are no characters in the keyboard
 *  buffer
 *
 *  @return The next character in the keyboard buffer, or -1 if the keyboard
 *          buffer is currently empty
 **/
int readchar() {
    return get_nextchar();
}

/** @brief get the nextline 
 *
 * @param buf user buffer to put the line in
 * @param len number of bytes
 *
 * @return int 0 if success and -ve integer on failure
 */
int nextline(char *buf, int len) {
    return get_nextline(buf, len);
}
