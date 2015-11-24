/** @file timer_handler.S
 *  @brief defines the timer interrupt handler
 *
 *  Pushes general registers onto stack and calls the c function
 *  which handles the timer interrupt
 * 
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */ 

.global timer_handler 

timer_handler:
    pusha                   /* Push all general purpose registers */
    call callback_handler   /* Call our callback function */
    popa                    /* Pop all general purpose registers */
    iret                    /* Return from interrupt */
