/** @file circular_buffer.c
 *  @brief implements a circular buffer
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <assert.h>
#include <errors.h>
#include <circular_buffer.h>
#include <stddef.h>
#include <simics.h>

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
void add_message(message_struct_t *msg_data, char msg) {

	assert(msg_data != NULL);

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
 *  @return char value of message if available. O
 */
char get_nextmsg(message_struct_t *msg_data) {
	if(msg_data == NULL) {
		return 0;
	}

	if(msg_data->start_ptr == msg_data->end_ptr) {
		return 0;
	}	
    char msg = msg_data->buf[msg_data->start_ptr];
    msg_data->start_ptr = NEXT(msg_data->start_ptr);
    return msg;
}
