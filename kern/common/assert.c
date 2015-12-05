/** @file assert.c
 *
 *  Various assert functions 
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <simics.h>
#include <core/wait_vanish.h>
#include <syscalls/lifecycle_syscalls.h>
#include <syscalls/misc_syscalls.h>

#define die(expression)  \
	((void)((expression) ? 0 : (panic_exit("PANIC! `%s' at %s:%u." \
                                "Program will exit", expression, \
                                __FILE__, __LINE__))))
/*
 * This function is called by the assert() macro defined in assert.h;
 * it's also a nice simple general-purpose panic function.
 */
void panic(const char *fmt, ...)
{
	va_list vl;
	char buf[80];

	va_start(vl, fmt);
	vsnprintf(buf, sizeof (buf), fmt, vl);
	va_end(vl);
	lprintf(buf);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");

	volatile static int side_effect = 0;
    // exact authorship uncertain, popularized by Heinlein
    printf("When in danger or in doubt, run in circles, scream and shout.\n");
    lprintf("When in danger or in doubt, run in circles, scream and shout.");
    ++side_effect;
}

/** @brief a panicking kernel
 *
 *  kill kernel
 */
void kernel_panic(const char *fmt, ...)
{
	va_list vl;
	char buf[80];

	va_start(vl, fmt);
	vsnprintf(buf, sizeof (buf), fmt, vl);
	va_end(vl);
	lprintf(buf);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");
	lprintf("KERNEL_PANIC! Going to kill kernel!: %s", fmt);
    halt_handler();
}

/** @brief a panicking thread
 *
 *  kill thread
 */
void thread_panic(const char *fmt, ...)
{
	va_list vl;
	char buf[80];

	va_start(vl, fmt);
	vsnprintf(buf, sizeof (buf), fmt, vl);
	va_end(vl);
	lprintf(buf);

	va_start(vl, fmt);
	vprintf(fmt, vl);
	va_end(vl);
	printf("\n");
	lprintf("THREAD_PANIC! Going to kill thread!: %s", fmt);
    set_status_handler_c(-2);
    do_vanish();
}
