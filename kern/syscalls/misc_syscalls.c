/** @file misc_syscalls.c
 *
 *  @brief implementation of miscellaneous system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <vm/vm.h>
#include <syscalls/syscall_util.h>
#include <loader/loader.h>
#include <common/errors.h>
#include <simics.h>
#define MAX_FILE_NAME 128

/** @brief read a file
 *
 * @param arg_packet
 * @return int 0 on success -ve integer on failure
 */
int readfile_handler_c(void *arg_packet) {
    char *filename = (char *)(*((int *)arg_packet));
    int count;
    for (count = 0; count < MAX_FILE_NAME; count++) {
        if (is_pointer_valid(&filename[count], 1) < 0) {
            return ERR_INVAL;
        }
        if (filename[count] == '\0') {
            break;
        }
    }
    if (count == 0 || count == MAX_FILE_NAME) {
        return ERR_FAILURE;
    }

    char *buf = (char *)(*((int *)arg_packet + 1));
    int size = (int)(*((int *)arg_packet + 2));
    int offset = (int)(*((int *)arg_packet + 3));
    int bytes_read = getbytes(filename, offset, size, buf);

    return bytes_read;
}

