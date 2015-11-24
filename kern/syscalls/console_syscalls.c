/** @file console_syscalls.c
 *
 *  @brief implementation of functions for console I/O
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <syscalls/console_syscalls.h>
#include <console.h>
#include <simics.h>
#include <core/scheduler.h>
#include <core/thread.h>
#include <drivers/keyboard/keyboard.h>
#include <common/errors.h>
#include <stdio.h>
#include <syscalls/syscall_util.h>
#include <vm/vm.h>

/** @brief print to screen 
 *
 *  @param arg_packet pointer to a memory location containing the arguments
 *  @return int 0 on success, -ve integer on failure
 */
int print_handler_c(void *arg_packet) {
    int len = *(int *)arg_packet;
    char *buf = (char *)(*((int *)arg_packet + 1));
    if (is_pointer_valid(buf, len) < 0) {
        return ERR_INVAL;
    }
    putbytes(buf, len);
    return 0;
}

/** @brief read a line from console
 *
 *  @param arg_packet pointer to memory location containing the arguments
 *  @return int number of bytes copied into the buffer
 */
int readline_handler_c(void *arg_packet) {
    int len = *(int *)arg_packet;
    if (len <= 0) {
        return ERR_INVAL;
    }
    char *buf = (char *)(*((int *)arg_packet + 1));
    if (is_pointer_valid(buf, len) < 0
        || is_memory_writable(buf, len) < 0) {
        return ERR_INVAL;
    }
    thread_struct_t *curr_thread = get_curr_thread();

    mutex_lock(&readline_mutex);
    int retval = nextline(buf, len);
    if (retval == ERR_INVAL) {
		mutex_unlock(&readline_mutex);
        return ERR_INVAL;
    }
    while (retval == ERR_NOTAVAIL) {
        cond_wait(&readline_cond_var, &readline_mutex, 
                  &curr_thread->cond_wait_link, WAITING);
        retval = nextline(buf, len);
    }
    mutex_unlock(&readline_mutex);
    return retval;
}

/** @brief read a character from console
 *
 *  @return char character that is read
 */
int getchar_handler_c() {
    thread_struct_t *curr_thread = get_curr_thread();

    mutex_lock(&readline_mutex);
    int c = readchar();
    while (c == ERR_NOTAVAIL) {
        cond_wait(&readline_cond_var, &readline_mutex, 
                  &curr_thread->cond_wait_link, WAITING);
        c = readchar();
    }
    mutex_unlock(&readline_mutex);
    return c;
}

/** @brief set the terminal color
 *
 *  @param color the color to be set
 *  @return int 0 on success, -ve integer on failure
 */
int set_term_color_handler_c(int color) {
    return set_term_color(color);
}

/** @brief set the cursor position
 *
 *  @param row the row to set the cursor position
 *  @param col the column to set the cursor position
 *  @return int 0 on success, -ve integer on failure
 */
int set_cursor_pos_handler_c(void *arg_packet) {
    int row = *(int *)arg_packet;
    int col = *((int *)arg_packet + 1);
    return set_cursor(row, col);
}

/** @brief get the cursor position
 *
 *  @param row the memory location to store row info
 *  @param col the memory location to store col info
 *  @return int 0 on success, -ve integer on failure
 */
int get_cursor_pos_handler_c(void *arg_packet) {
    int *row = (int *)(*(int *)arg_packet);
    if (is_pointer_valid(row, sizeof(int)) < 0
        || is_memory_writable(row, sizeof(int)) < 0) {
        return ERR_INVAL;
    }
    int *col = (int *)(*((int *)arg_packet + 1));
    if (is_pointer_valid(col, sizeof(int)) < 0
        || is_memory_writable(col, sizeof(int)) < 0) {
        return ERR_INVAL;
    }
    get_cursor(row, col);
    return 0;
}

