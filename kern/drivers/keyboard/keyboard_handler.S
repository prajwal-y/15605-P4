/** @file keyboard_handler.S
 *  @brief defines the keyboard interrupt handler
 *
 *  Pushes general registers onto stack and calls the c function
 *  which handles the keyboard interrupt
 * 
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */ 

.global keyboard_handler 

keyboard_handler:
    pusha                   /* Push all general purpose registers */
    call enqueue_scancode   /* Call our callback function */
    popa                    /* Pop all general purpose registers */
    iret                    /* Return from interrupt */
