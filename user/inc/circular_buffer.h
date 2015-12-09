/** @file circular_buffer.h
 *  @brief implements a circular buffer
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __USER_CIRCULAR_BUFFER_H
#define __USER_CIRCULAR_BUFFER_H

#include <syscall.h>

#define BUFFER_SIZE 1000

typedef struct message_struct {
	char buf[BUFFER_SIZE];
	int start_ptr;
	int end_ptr;
} message_struct_t;

void init_msg_data(message_struct_t *msg_data);

int has_message(message_struct_t *msg_data);

void add_message(message_struct_t *msg_data, char msg);

char get_nextmsg(message_struct_t *msg_data);
 
#endif  /* __USER_CIRCULAR_BUFFER_H */
