/** @file circular_buffer.h
 *  @brief implements a circular buffer for keystrokes
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#define KEYBOARD_BUFFER_SIZE 2000

#define ERR_INVAL -1
#define ERR_NOTAVAIL -2

void add_keystroke(char c);

int get_nextline(char *buf, int len);

int get_nextchar();
 
#endif  /* __CIRCULAR_BUFFER_H */
