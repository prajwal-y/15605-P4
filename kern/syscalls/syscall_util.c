/** @file syscall_util.c
 *
 *  Util functions for syscalls 
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <ureg.h>
#include <stdlib.h>
#include <core/scheduler.h>
#include <core/thread.h>
#include <seg.h>
#include <simics.h>
#include <asm/asm.h>
#include <x86/eflags.h>
#include <common/errors.h>
#include <vm/vm.h>
#include <common_kern.h>
#include <syscalls/syscall_util.h>

static int validate_uregs(ureg_t *uregs);

/** @brief setup the kernel stack
 *
 *  @param pointer to a ureg area containing new values to be applied
 *  @param kernel_stack_base the base of the kernel stack
 *  @return 0 on success, -ve integer on failure
 */
int setup_kernel_stack(ureg_t *ureg, void *kernel_stack_base) {
    int retval = validate_uregs(ureg);
    if (retval < 0) {
        return retval;
    }
    *((int *)(kernel_stack_base) - 1) = ureg->ds;
    *((int *)(kernel_stack_base) - 2) = ureg->esp;
    *((int *)(kernel_stack_base) - 3) = ureg->eflags;
    *((int *)(kernel_stack_base) - 4) = ureg->cs;
    *((int *)(kernel_stack_base) - 5) = ureg->eip;

    /* Simulate a SAVE_REGS for the 8 registers that are pushed */
    *((int *)(kernel_stack_base) - 6) = ureg->ecx;
    *((int *)(kernel_stack_base) - 7) = ureg->eax;
    *((int *)(kernel_stack_base) - 8) = ureg->edx;
    *((int *)(kernel_stack_base) - 9) = ureg->ebx;
    *((int *)(kernel_stack_base) - 11) = ureg->ebp;
    *((int *)(kernel_stack_base) - 12) = ureg->edi;
    *((int *)(kernel_stack_base) - 13) = ureg->esi;

    return 0;
}

/** @brief validate the uregs passed in by the user for swexn
 *
 *  @return 0 on success, -ve integer on failure
 */
int validate_uregs(ureg_t *uregs) {
	/* IOPL shouldn't be user privilege */
    if ((uregs->eflags & EFLAGS_IOPL) == EFLAGS_IOPL) {
        return ERR_INVAL;
    }
    /* USer should not be allowed to modify interrupt flags */
    if ((uregs->eflags & EFLAGS_IF) != EFLAGS_IF) {
        return ERR_INVAL;
    }
    return 0;
}

/** @brief populate an ureg struct using the current values on the exception stack
 *
 *  @param ureg a pointer to the ureg struct to be filled in
 *  @param err_code_avail whether an error code is available on the stack
 *
 *  @return void
 */
void populate_ureg(ureg_t *ureg, int err_code_avail, 
                   thread_struct_t *curr_thread) {
    void *kernel_stack_base = (void *)curr_thread->k_stack_base;

    ureg->ds = SEGSEL_USER_DS;
    ureg->es = SEGSEL_USER_DS;
    ureg->fs = SEGSEL_USER_DS;
    ureg->gs = SEGSEL_USER_DS;

    ureg->eax = *((int *)(kernel_stack_base) - 6 - err_code_avail);
    ureg->ecx = *((int *)(kernel_stack_base) - 7 - err_code_avail);
    ureg->edx = *((int *)(kernel_stack_base) - 8 - err_code_avail);
    ureg->ebx = *((int *)(kernel_stack_base) - 9 - err_code_avail);
    ureg->zero = 0;
    ureg->ebp = *((int *)(kernel_stack_base) - 11 - err_code_avail);
    ureg->esi = *((int *)(kernel_stack_base) - 12 - err_code_avail);
    ureg->edi = *((int *)(kernel_stack_base) - 13 - err_code_avail);

    if (err_code_avail == ERR_CODE_AVAIL) {
        ureg->error_code = *((int *)(kernel_stack_base) - 6);
    }
    else {
        ureg->error_code = 0;
    }

    ureg->eip = *((int *)(kernel_stack_base) - 5);
    ureg->cs = *((int *)(kernel_stack_base) - 4);
    ureg->eflags = *((int *)(kernel_stack_base) - 3);
    ureg->esp = *((int *)(kernel_stack_base) - 2);
    ureg->ss = *((int *)(kernel_stack_base) - 1);
}

/** @brief check if address is (valid) mapped in user space
 *
 *  return error if either the address is in kernel space or
 *  is unmapped in user space
 *
 *  @param ptr address to check
 *  @param bytes number of bytes that have to be checked
 *  @return 0 if mapped and safe, -ve integer if not
 */
int is_pointer_valid(void *ptr, int bytes) {
    if (ptr < (void *)USER_MEM_START) {
        return ERR_INVAL;
    }
    if (is_memory_range_mapped(ptr, bytes) == MEMORY_REGION_UNMAPPED) {
        return ERR_INVAL;
    }
    return 0;
}

/** @brief copy user memory to kernel memory checking for validity
 *
 *  Check validity of each byte and if valid copy into kernel buf. If
 *  data exceeds max_size return ERR_BIG. It is expected that the caller
 *  of this function will allocate atleast max_size memory for buf. This
 *  is a reasonable assumption to make since this function will be used only
 *  by kernel code.
 *  
 *  @param buf kernel memory to copy into
 *  @param ptr user ptr to copy from
 *  @param max_size the maximum amount of data in bytes to copy
 *
 *  @return bytes copied on success, -ve integer on failure 
 */
int copy_user_data(char *buf, char *ptr, int max_size) {
    if (buf == NULL || ptr == NULL || max_size <= 0) {
        return ERR_INVAL;
    }

    int count = 0;

    while (count < max_size) {
        if (is_pointer_valid(&ptr[count], 1) < 0) {
            return ERR_INVAL;
        }
        buf[count] = ptr[count];
        if (buf[count] == '\0') {
            break;
        }
        count++;
    }
    if (count == max_size) {
        return ERR_BIG;
    }
    return count + 1;
}
