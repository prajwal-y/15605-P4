/** @file circular_buffer.h
 *  @brief implements a circular buffer for storing
 *  interrupt messages
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __CIRCULAR_BUFFER_H
#define __CIRCULAR_BUFFER_H

#include <syscall.h>

#define BUFFER_SIZE 1000

typedef struct message_struct {
	message_t buf[BUFFER_SIZE];
	int start_ptr;
	int end_ptr;
} message_struct_t;

void init_msg_data(message_struct_t *msg_data);

int has_message(message_struct_t *msg_data);

void add_message(message_struct_t *msg_data, message_t msg);

message_t get_nextmsg(message_struct_t *msg_data);
 
#endif  /* __CIRCULAR_BUFFER_H */
