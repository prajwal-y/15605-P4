/** @file mutex.c
 *  @brief Implementation of mutex calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <asm.h>
#include <sync/mutex.h>
#include <list/list.h>
#include <core/thread.h>
#include <core/scheduler.h>
#include <core/context.h>
#include <syscall.h>
#include <simics.h>
#include <common/assert.h>
#include <common/errors.h>
#include <eflags.h>

#define EFLAGS_IF 0x00000200 

static int enable = 0;
static void disable_interrupts_mutex();
static void enable_interrupts_mutex();

/** @brief initialize a mutex
 *
 *  Set the mutex value to 1 indicating that it is unlocked
 *  and initialize the wait queue.
 *
 *  @return 0 on success and -1 for invalid input
 */
int mutex_init(mutex_t *mp) {
    if (mp == NULL) {
        return ERR_INVAL;
    }
    mp->value = MUTEX_VALID;
	init_head(&mp->waiting);
    return 0;
}

/** @brief Function to allow enabling interrupts in mutex lock
 *  and unlock. This function is called before the bootstrap
 *  task is about to enter user space
 *
 *  @return void
 */
void enable_mutex_lib() {
	enable = 1;
}

/** @brief Wrapper for disabling interrupts.
 *
 *  Before the first task is loaded, there is only
 *  one thread running, and we do not disable interrupts.
 *
 *  @return void
 */
void disable_interrupts_mutex() {
	if(enable) {
		disable_interrupts();
	}
}

/** @brief Wrapper for enabling interrupts.
 *
 *  Before the first task is loaded, there is only
 *  one thread running, and we do not enable interrupts.
 *
 *  @return void
 */
void enable_interrupts_mutex() {
	if(enable) {
		enable_interrupts();
	}
}

/** @brief destroy a mutex
 *
 *  Sets the value of the mutex to -1 indicating that it is inactive.
 *  This function does not reclaim the space used by the mutex.
 *
 *  @return void
 */
void mutex_destroy(mutex_t *mp) {
	thread_assert(mp != NULL);
	thread_assert(mp->value != MUTEX_INVALID);
    mp->value = MUTEX_INVALID;
}

/** @brief attempt to acquire the lock
 *
 *  This function disables interrupts to check for the lock.
 *  If the lock is present, then the value of the lock is
 *  0 (lock is aquired), and the function returns after enabling 
 *  interrupts. Otherwise, the thread is added to waiting queue
 *  and context_switch() is called.
 *
 *  @param mp the mutex to be locked
 *  @return void
 */
void mutex_lock(mutex_t *mp) {
	thread_assert(mp != NULL);
	thread_assert(mp->value != MUTEX_INVALID);
	disable_interrupts_mutex();
	while(mp->value == 0) {
		thread_struct_t *curr_thread = get_curr_thread();
		curr_thread->status = WAITING;
		add_to_tail(&curr_thread->mutex_link, &mp->waiting);
		context_switch();
		disable_interrupts_mutex();
	}
	mp->value = 0;
	enable_interrupts_mutex();
}

/** @brief release a lock
 *
 *  When the lock is released, the first thread
 *  in the waiting queue is made runnable and the value of the
 *  mutex is set to 1.
 *  
 *  @param mp the mutex to be unlocked
 *  @return void
 */
void mutex_unlock(mutex_t *mp) {
	thread_assert(mp != NULL);
	thread_assert(mp->value != MUTEX_INVALID);
    disable_interrupts_mutex();
	list_head *waiting_thread = get_first(&mp->waiting);
	if(waiting_thread != NULL) {
		thread_struct_t *thr = get_entry(waiting_thread, thread_struct_t,
                                          mutex_link);
		thr->status = RUNNABLE;
		runq_add_thread_interruptible(thr);
		del_entry(&thr->mutex_link);
	}
	mp->value = 1;
	enable_interrupts_mutex();
}

/** @brief attempt to acquire the lock and keep the original state of interrupts
 *
 *  This function disables interrupts to check for the lock.
 *  If the lock is present, then the value of the lock is
 *  0 (lock is aquired), and the function returns after enabling 
 *  interrupts. Otherwise, the thread is added to waiting queue
 *  and context_switch() is called. When it returns we enable interrupts
 *  only if it was previously enabled. Currently this is used only by condition
 *  variables and sfree() which are called in a interrupt disabled scenario
 *  from vanish(). It can be extended to be used in other places as well.
 *
 *  @param mp the mutex to be locked
 *  @return void
 */
void mutex_lock_int_save(mutex_t *mp) {
	thread_assert(mp != NULL);
	thread_assert(mp->value != MUTEX_INVALID);
    int int_flag = get_eflags() & EFLAGS_IF;

	disable_interrupts_mutex();
	while(mp->value == 0) {
		thread_struct_t *curr_thread = get_curr_thread();
		curr_thread->status = WAITING;
		add_to_tail(&curr_thread->mutex_link, &mp->waiting);
		context_switch();
		disable_interrupts_mutex();
	}
	mp->value = 0;
    if (int_flag) {
    	enable_interrupts_mutex();
    }
}

/** @brief release a lock and keep the original state of interrupts
 *
 *  When the lock is released, the first thread
 *  in the waiting queue is made runnable and the value of the
 *  mutex is set to 1. We enable interrupts only if it was previously enabled.
 *  Currently this is used only by condition variables and sfree() which are 
 *  called in a interrupt disabled scenario from vanish(). It can be extended 
 *  to be used in other places as well.
 *
 *  @param mp the mutex to be unlocked
 *  @return void
 */
void mutex_unlock_int_save(mutex_t *mp) {
	thread_assert(mp != NULL);
	thread_assert(mp->value != MUTEX_INVALID);
    int int_flag = get_eflags() & EFLAGS_IF;

    disable_interrupts_mutex();
	list_head *waiting_thread = get_first(&mp->waiting);
	if(waiting_thread != NULL) {
		thread_struct_t *thr = get_entry(waiting_thread, thread_struct_t,
                                          mutex_link);
		thr->status = RUNNABLE;
		runq_add_thread_interruptible(thr);
		del_entry(&thr->mutex_link);
	}
	mp->value = 1;
    if (int_flag) {
    	enable_interrupts_mutex();
    }
}
