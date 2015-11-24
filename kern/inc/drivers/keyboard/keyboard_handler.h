/** @file keyboard_handler.h
 *  @brief prototype for functions in keyboard_handler.S
 *   
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __KEYBOARD_HANDLER_H
#define __KEYBOARD_HANDLER_H

/** @brief the keyboard interrupt handler
 *
 *  This function is called whenever a key is pressed.
 *  It pushes the general purpose registers onto the stack, calls the 
 *  C function responsible for handling the interrupt and then pops the
 *  general registers and returns.
 */
void keyboard_handler(); 

#endif  /* __KEYBOARD_HANDLER_H */
