/** @file memory_syscalls.c
 *
 *  @brief implementation of functions for thread management functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <syscalls/memory_syscalls.h>
#include <vm/vm.h>
#include <syscall.h>
#include <common/errors.h>
#include <simics.h>
#include <core/thread.h>

/** @brief Handler to call the new_pages handler function
 *
 *  @return int 0 on success, -ve integer on failure
 */
int new_pages_handler_c(void *arg_packet) {
    void *base = (void *)(*(int *)arg_packet);
    int len = (int)(*((int *)arg_packet + 1));  
    if (len <= 0 || (len % PAGE_SIZE) != 0 || ((int)base % PAGE_SIZE) != 0) {
        return ERR_INVAL;
    }
    if (is_memory_range_mapped(base, len) != MEMORY_REGION_UNMAPPED) {
        return ERR_INVAL;
    }
    
    return map_new_pages(base, len);
}

/** @brief Handler to call the remove_pages handler function
 *
 *  @return int 0 on success, -ve integer on failure
 */
int remove_pages_handler_c(void *base) {
    if (((int)base % PAGE_SIZE) != 0) {
        return ERR_INVAL;
    }
    
    return unmap_new_pages(base);
}
