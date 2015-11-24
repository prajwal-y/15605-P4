/** @file assert.h
 *
 *  @brief Macros for asserts used in the P3 kernel
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef _ASSERT_H_
#define _ASSERT_H_

void panic_exit(const char *fmt, ...);
void panic(const char *format, ...);
void kernel_panic(const char *format, ...);
void thread_panic(const char *format, ...);

#define assert(expression)  \
	((void)((expression) ? 0 : (panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#define thread_assert(expression)  \
	((void)((expression) ? 0 : (thread_panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#define kernel_assert(expression)  \
	((void)((expression) ? 0 : (kernel_panic("%s:%u: failed assertion `%s'", \
					  __FILE__, __LINE__, #expression), 0)))

#endif /* _ASSERT_H_ */
