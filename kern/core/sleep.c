/** @file sleep.c
 *
 *  File which implements the functions required for sleep
 *  system call.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <core/scheduler.h>
#include <common/errors.h>
#include <common/assert.h>
#include <simics.h>
#include <list/list.h>
#include <drivers/timer/timer.h>
#include <core/context.h>
#include <core/thread.h>
#include <sync/mutex.h> 
#include <asm.h>
#include <common/malloc_wrappers.h>

static list_head *sleeping_threads;    /* List of threads that are 'sleep'ing */

static mutex_t sleep_list_mutex;

/** @brief Function to initialize the list of sleeping threads
 *
 *  @return void
 */
void init_sleeping_threads() {
    mutex_init(&sleep_list_mutex);
    sleeping_threads = (list_head *)smalloc(sizeof(list_head));
    kernel_assert(sleeping_threads != NULL);
    init_head(sleeping_threads);
}

/** @brief The entry point for sleep
 *
 *  @return int 0 if succesful, -ve integer on failure
 */
int do_sleep(int ticks) {
    if (ticks == 0) {
        return 0;
    }
    if (ticks < 0) {
        return ERR_INVAL;
    }
    
    schedule_sleep(ticks);

    return 0;

}

/** @brief return next sleeping thread ready to be woken if any
 *
 *  Check if the first sleeping thread is ready to be woken up (based on
 *  the current ticks and the first thread's wake time). If any thread is
 *  to be woken up, return that. Else return NULL.
 *
 *  @return thread_struct_t NULL if no thread existing or ready to be woken up
 */
thread_struct_t *get_sleeping_thread() {
    /* Check if any sleeping thread can be woken up */
    if (sleeping_threads != NULL) {
        list_head *thr_entry = get_first(sleeping_threads);
        if (thr_entry != NULL) {
            int curr_ticks = total_ticks();
            thread_struct_t *thr = get_entry(thr_entry, thread_struct_t, 
                                             sleepq_link);
            if (thr->wake_time < curr_ticks) {
                del_entry(&thr->sleepq_link);
                return thr;
            }
        }
    }
    return NULL;
}


/** @brief schedule a thread for sleeping
 *
 *  calculate the time at which the current thread will be woken up given
 *  the number of ticks that the thread has to sleep. Then obtain a lock 
 *  on the sleeping thread queue, temporarily set the sleeping thread head to 
 *  NULL (so that the scheduler can run in parallel and not interfere with 
 *  the code to maintain the list in sorted order). Then we find the right 
 *  position for this thread based on the time it has to wake up, with the 
 *  head of the queue being the earliest wake time.
 *
 *  @param ticks number of ticks to sleep for
 *  @return void
 */
void schedule_sleep(int ticks) {
    thread_struct_t *thr = get_curr_thread();
    int curr_ticks = total_ticks();

	/* In case the integer overflow, we don't sleep */
	if(curr_ticks + ticks < curr_ticks) {
		return;
	}

    thr->wake_time = curr_ticks + ticks;

    mutex_lock(&sleep_list_mutex);
    list_head *temp_head = sleeping_threads;
    sleeping_threads = NULL;
    list_head *entry = get_first(temp_head);
    while (entry != NULL && entry != temp_head) {
        thread_struct_t *thread = get_entry(entry, thread_struct_t, 
                                            sleepq_link);
        if (thread->wake_time > thr->wake_time) {
            break;
        }
        entry = entry->next;
    }
    if (entry == NULL) {
        add_to_tail(&thr->sleepq_link, temp_head);
    }
    else {
        add_to_list(&thr->sleepq_link, entry->prev, entry);
    }
    mutex_unlock(&sleep_list_mutex);
    disable_interrupts();
    sleeping_threads = temp_head;
    thr->status = WAITING;
    context_switch();
}
