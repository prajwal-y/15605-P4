/** @file keyboard_circular_buffer.h
 *  @brief implements a circular buffer for keystrokes
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __KEYBOARD_CIRCULAR_BUFFER_H
#define __KEYBOARD_CIRCULAR_BUFFER_H
#define KEYBOARD_BUFFER_SIZE 2000

void add_keystroke(char c);

int get_nextline(char *buf, int len);

int get_nextchar();
 
#endif  /* __KEYBOARD_CIRCULAR_BUFFER_H */
