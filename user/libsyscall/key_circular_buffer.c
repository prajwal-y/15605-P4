/** @file keyboard_circular_buffer.c
 *  @brief implements a circular buffer for keystrokes
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <key_circular_buffer.h>
#include <syscall.h>
#include <asm.h>
#include <stddef.h>

#define NEXT(i) (((i) + 1) % (KEYBOARD_BUFFER_SIZE))
#define PREV(i) (((i) == 0) ? (KEYBOARD_BUFFER_SIZE - 1) : ((i) - 1))
#define NOT_PRESENT -1

static int key_buf[KEYBOARD_BUFFER_SIZE];
static void update_newline_ptr();
static void update_start_ptr();

int start_ptr = 0;
int end_ptr = 0;
int newline_ptr = -1;

/** @brief add keystroke to buffer
 *
 *  @param c next character
 *  @return void
 */
void add_keystroke(char c) {
    /* Check if buffer is full */
    if (NEXT(end_ptr) == start_ptr) {
        return;
    }
    key_buf[end_ptr] = c;
    if (c == '\n' && newline_ptr == -1) {
        newline_ptr = end_ptr;
    }
    if (c == '\b') {
		if(end_ptr != start_ptr) {
        	end_ptr = PREV(end_ptr);
		}
		char buf[1] = {c};
		print(1, buf);
        return;
    }
	char buf[1] = {c};
	print(1, buf);
    end_ptr = NEXT(end_ptr);
}

/** @brief get the nextline 
 *
 * @param buf user buffer to put the line in
 * @param len number of bytes
 *
 * @return int number of characters in the line and -ve integer on failure
 */
int get_nextline(char *buf, int len) {
    if (buf == NULL || len >= KEYBOARD_BUFFER_SIZE) {
        return ERR_INVAL;
    }
    int num_bytes = 0;

    if (newline_ptr == -1) {
       return ERR_NOTAVAIL;
    }
    
    int i;
    for (i = 0; i < len; i++) {
        if (start_ptr == newline_ptr) {
			buf[i] = '\n';
            break;
        }
        buf[i] = key_buf[start_ptr];
        start_ptr = NEXT(start_ptr);
        num_bytes++;
    }
    update_start_ptr();
    update_newline_ptr();
    return num_bytes;
}

/** @brief get the next character 
 *
 * @return int character from console, -ve integer if non available
 */
int get_nextchar() {
    if (start_ptr == end_ptr) {
       return ERR_NOTAVAIL;
    }
    
    int c = key_buf[start_ptr];
    start_ptr++;
    return c;
}

/** @brief update start_ptr
 *
 *  @pre must be called before update_newline_ptr
 *
 *  @return void
 */
void update_start_ptr() {
    while (start_ptr != newline_ptr) {
        start_ptr = NEXT(start_ptr);
    }
    start_ptr = NEXT(start_ptr);
}

/** @brief update newline ptr
 *
 *  @return void
 */
void update_newline_ptr() {
    newline_ptr = NEXT(newline_ptr);
    while (key_buf[newline_ptr] != '\n') {
        if (newline_ptr == end_ptr) {
            newline_ptr = NOT_PRESENT;
            return;
        }
        newline_ptr = NEXT(newline_ptr);
    }
}
