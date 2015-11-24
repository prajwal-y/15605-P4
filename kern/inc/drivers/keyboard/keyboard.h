/** @file keyboard.h
 *  @brief Prototypes of functions for the keyboard 
 *         driver
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __KEYBOARD_H
#define __KEYBOARD_H
#include <sync/cond_var.h>
#include <sync/mutex.h>

extern cond_t readline_cond_var;

extern mutex_t readline_mutex;

int install_keyboard_handler();

int readchar();

int nextline(char *buf, int len);

#endif  /* __KEYBOARD_H */
