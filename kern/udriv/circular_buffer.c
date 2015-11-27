/** @file circular_buffer.c
 *  @brief implements a circular buffer to store
 *  interrupt messages
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <console.h>
#include <common/assert.h>
#include <common/errors.h>
#include <udriv/udriv.h>
#include <udriv/circular_buffer.h>
#include <stddef.h>

#define NEXT(i) (((i) + 1) % (BUFFER_SIZE))
#define PREV(i) (((i) == 0) ? (BUFFER_SIZE - 1) : ((i) - 1))

/** @brief Initialize a message struct
 * 
 *  @param msg_data The message struct to be initialized
 *
 *  @return void
 */
void init_msg_data(message_struct_t *msg_data) {
	if(msg_data == NULL) {
		return;
	}
	msg_data->start_ptr = 0;
	msg_data->end_ptr = 0;
}

/** @brief adds a message to buffer
 *
 *  @param msg_data The information of the buffer to be filled
 *  @param msg The message to be stored
 *
 *  @return void
 */
void add_message(message_struct_t *msg_data, message_t msg) {

	thread_assert(msg_data != NULL);

    /* Check if buffer is full */
    if (NEXT(msg_data->end_ptr) == msg_data->start_ptr) {
        return;	// We will drop messages until buffer has a free space
    }
    msg_data->buf[msg_data->end_ptr] = msg;
    msg_data->end_ptr = NEXT(msg_data->end_ptr);
}   

/** @brief Function to check if a message is present in the
 *  message buffer of a driver
 *
 *  @param msg_data The information of the buffer to be checked
 *
 *  @return 1 if message is present. 0 if not
 */
int has_message(message_struct_t *msg_data) {
	if(msg_data == NULL) {
		return 0;
	}
	if(msg_data->start_ptr == msg_data->end_ptr) {
		return 0;
	}
	return 1;
}

/** @brief get the next message from the buffer
 *
 *  @param msg_data The information of the buffer to be checked
 *
 *  @return message_t value of message if available. O
 */
message_t get_nextmsg(message_struct_t *msg_data) {
	if(msg_data == NULL) {
		return 0;
	}

	if(msg_data->start_ptr == msg_data->end_ptr) {
		return 0;
	}	
    message_t msg = msg_data->buf[msg_data->start_ptr];
    msg_data->start_ptr++;
    return msg;
}
