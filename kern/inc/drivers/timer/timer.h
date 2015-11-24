/**@file timer.h 
 * @brief a file containing prototype definitions for timer
 *         driver functions
 *
 * @author Rohit Upadhyaya (rjupadhy)
 * @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef __TIMER_H
#define __TIMER_H

int initialize_timer(void (*tickback)(unsigned int));

void callback_handler();

unsigned int total_ticks();

#endif  /* __TIMER_H */
