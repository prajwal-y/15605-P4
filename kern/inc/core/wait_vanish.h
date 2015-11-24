/** @file wait_vanish.h
 *
 *  Prototypes for wait and vanish system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __WAIT_VANISH_H
#define __WAIT_VANISH_H

int do_wait(void *arg_packet);
void do_vanish();

#endif  /* __WAIT_VANISH_H */
