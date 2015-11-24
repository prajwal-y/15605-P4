/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 *  @bug No known bugs.
 */

#include <common_kern.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */
#include <x86/seg.h>                /* install_user_segs() */
#include <interrupts/interrupt_handlers.h>  
#include <drivers/console/console_util.h>
#include <common/malloc_wrappers.h>
#include <common/assert.h>
#include <console.h>
#include <allocator/frame_allocator.h>
#include <x86/cr.h>
#include <vm/vm.h>
#include <simics.h>
#include <loader/loader.h>
#include <core/thread.h>
#include <core/task.h>
#include <exec2obj.h>
#include <core/scheduler.h>
#include <syscalls/syscall_handlers.h>

static void set_default_color();

/** @brief Kernel entrypoint.
 *  
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{

	/* Initialize the thread safe malloc library */
	init_thr_safe_malloc_lib();

    /* Initialize the VM system */
    vm_init();

    /* Set defaulr console color */
    set_default_color();

    /* Install all the fault handlers we expect to use */
    kernel_assert(install_handlers() == 0);

	/* Install the system call handlers */
    kernel_assert(install_syscall_handlers() == 0);

    /* Clear the console of crud */
    clear_console();

    /* Initialize user space physical frame allocator */
    init_frame_allocator();

    /* Initialize scheduler system */
    init_scheduler();

    /* Initialize kernel threads subsystem */
    kernel_threads_init();

    /* Load the init task into memory. This does NOT make the init task 
     * runnable. This is taken care of by the scheduler/context switcher */
	load_init_task("init");

    /* Load the idle task in user mode. This task ALWAYS has TID 1 */
    load_bootstrap_task("idle");

    /* Should never come here */
    while (1) {
        continue;
    }

    return 0;
}

/** @brief function to set the default color of the console
 *
 *  By default the console will have a background color of black (0x00)
 *  and foreground color of white (0xF). This function also sets a 2 byte
 *  array to the empty character which is used for backspace and for scrolling
 *  among other things. This 2 byte array is memcpy'ed into video memory locations
 *  where an empty character is required.
 *
 *  @return void
 */
void set_default_color() {
    console_color = BGND_BLACK | FGND_WHITE;
    empty_char[0] = ' ';
    empty_char[1] = console_color;
}
