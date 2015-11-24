/** @file timer_handler.h
 *  @brief prototype for functions in timer_handler.S
 *   
 *  @author Rohit Upadhyaya (rjupadhy)
 */
#ifndef __TIMER_HANDLER_H
#define __TIMER_HANDLER_H

/** @brief the timer interrupt handler
 *
 *  This function is called whenever a timer interrupt is triggered.
 *  It pushes the general purpose registers onto the stack, calls the 
 *  C function responsible for handling the interrupt and then pops the
 *  general registers and returns.
 */
void timer_handler(); 

#endif  /* __TIMER_HANDLER_H */
