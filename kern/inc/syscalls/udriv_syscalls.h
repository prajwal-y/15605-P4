/** @file udriv_syscalls.h
 *
 *  @brief prototypes of functions for user drivers
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __UDRIV_SYSCALLS_H
#define __UDRIV_SYSCALLS_H

int udriv_register_handler();

void udriv_deregister_handler();

int udriv_send_handler();

int udriv_wait_handler();

int udriv_inb_handler();

int udriv_outb_handler();

int udriv_mmap_handler();

#endif  /* __UDRIV_SYSCALLS_H */
