/** @file system_check_syscalls.c
 *
 *  @brief implementation of functions for system check functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <syscalls/system_check_syscalls.h>
#include <vm/vm.h>
#include <syscall.h>
#include <common/errors.h>
#include <simics.h>
#include <allocator/frame_allocator.h>
#include <malloc_internal.h>

/** @brief Handler to perform basic memory check on kernel and 
 *         user space memory
 *
 *  @return void
 */
void memory_check_handler_c() {
    /* Check physical frame (count, etc) */
    check_physical_memory();

    /* Check kernel memory */
    lmm_dump(&malloc_lmm);
}
