/** @file console_util.h 
 *  @brief This file contains prototypes for various utility
 *         functions used by the console driver. The implementations
 *         of these prototypes are present in console_util.c
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __CONSOLE_UTIL_H
#define __CONSOLE_UTIL_H

void set_logical_cursor(int row, int col);

void scroll_screen(int num_rows);

void print_char(char ch, int row, int col, int color);

void empty_fill(void *addr, int num);

int is_valid_color(int color);

void set_hardware_cursor();

extern int cursor_row;

extern int cursor_col;

extern int is_cursor_hidden();

extern char console_color;

extern char empty_char[2];

#endif  /* __CONSOLE_UTIL_H */
