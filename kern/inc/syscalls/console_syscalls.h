/** @file console_syscalls.h
 *
 *  @brief prototypes of functions for console I/O
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __CONSOLE_SYSCALLS_H
#define __CONSOLE_SYSCALLS_H

int print_handler();

int print_handler_c(void *arg_packet);

int readline_handler();

int readline_handler_c(void *arg_packet);

int set_term_color_handler();

int set_term_color_handler_c(int color);

int set_cursor_pos_handler();

int set_cursor_pos_handler_c(void *arg_packet);

int get_cursor_pos_handler();

int get_cursor_pos_handler_c(void *arg_packet);

int getchar_handler();

int getchar_handler_c();

#endif  /* __CONSOLE_SYSCALLS_H */
