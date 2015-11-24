/** @file cond_var.c
 *
 *  @brief Implementation of sort of something like 
 *  condition variables, specifically designed for 
 *  waiting threads.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <sync/cond_var.h>
#include <asm.h>
#include <list/list.h>
#include <common/errors.h>
#include <common/assert.h>
#include <sync/mutex.h>
#include <core/thread.h>
#include <simics.h>
#include <core/context.h>
#include <core/scheduler.h>

/** @brief initialize a cond var
 *
 *  Set status of cond var to 1. It "initializes" the mutex pointed to 
 *  by cv. This also initializes the head of queue of waiting threads.
 *  Calling this function on an already initialized function can lead to
 *  undefined behavior.
 *
 *  @param cv a pointer to the condition variable
 *
 *  @return 0 on success and ERR_INVAL for invalid input
 */
int cond_init(cond_t *cv) {
    if (cv == NULL) {
        return ERR_INVAL;
    }
    cv->status = COND_VAR_VALID;
    if (mutex_init(&cv->queue_mutex) < 0) {
        return ERR_INVAL;
    }
    init_head(&cv->waiting);
    return 0;
}

/** @brief destroy a cond var
 *
 *  Sets the value of the status to COND_VAR_INVALID indicating that 
 *  it is inactive.
 *  This function does not reclaim the space used by the cond var.
 *  Once a cond var is destroyed calling cond var functions can lead to
 *  undefined behavior.
 *
 *  @param cv a pointer to the condition variable
 *  @return void
 */
void cond_destroy(cond_t *cv) {
	thread_assert(cv != NULL);
	thread_assert(cv->status != COND_VAR_INVALID);
    mutex_destroy(&cv->queue_mutex);
    cv->status = COND_VAR_INVALID;
}

/** @brief This function allows a thread to sleep on a signal issued on 
 *         some condition
 *
 *  This function adds thread to the queue of blocked threads and deschedules
 *  itself after unlocking the mutex associated with the cond var. When the 
 *  thread is woken up we acquire the mutex and return.
 *
 *  @pre the mutex pointed to by mp must be locked
 *  @post the mutex pointed to by mp is locked
 *
 *  @param cv a pointer to the condition variable
 *  @param mp a pointer to the mutex associated with the thread
 *  @param link a pointer to the "list_head" of the waiting list on this cv
 *  @param status the status that the thread should be set to
 *  @return void
 */
void cond_wait(cond_t *cv, mutex_t *mp, list_head *link, 
               int status) { 
	thread_assert(cv != NULL);
	thread_assert(cv->status != COND_VAR_INVALID);

    /* Protect accesses to the queue */
    mutex_lock(&cv->queue_mutex);
    add_to_tail(link, &cv->waiting);
    mutex_unlock(&cv->queue_mutex);

	disable_interrupts();
	thread_struct_t *curr_thread = get_curr_thread();
	curr_thread->status = status;

	/* Release the mutex and call context switch */
	mutex_unlock(mp);
	context_switch();

	/* Acquire the mutex again before returning */
	mutex_lock(mp);
}


/** @brief this function signals an event and wakes up a waiting thread
 *         if present
 *
 *  Get the first thread in the waiting queue and make it runnable. Deletes 
 *  it from the waiting queue.
 *
 *  @param cv a pointer to the condition variable
 *  @return void
 */
void cond_signal(cond_t *cv) {
	thread_assert(cv != NULL);
	thread_assert(cv->status != COND_VAR_INVALID);

	mutex_lock_int_save(&cv->queue_mutex); 
    list_head *waiting_thread = get_first(&cv->waiting);
    if (waiting_thread != NULL) {
        thread_struct_t *thr = get_entry(waiting_thread, thread_struct_t, 
                                          cond_wait_link);
		thr->status = RUNNABLE;
		runq_add_thread(thr);
		del_entry(&thr->cond_wait_link);
    }
    mutex_unlock_int_save(&cv->queue_mutex);
}

/** @brief this function signals all threads waiting on this cond var
 *  
 *  Makes all threads waiting on the cond var runnable
 *
 *  @param cv a pointer to the condition variable
 *  @return void
 */
void cond_broadcast(cond_t *cv) {
	thread_assert(cv != NULL);
	thread_assert(cv->status != COND_VAR_INVALID);
	
	mutex_lock_int_save(&cv->queue_mutex);
    list_head *waiting_thread = get_first(&cv->waiting);
	while(waiting_thread != NULL && waiting_thread != &cv->waiting) {
        thread_struct_t *thr = get_entry(waiting_thread, thread_struct_t, 
                                          cond_wait_link);
		thr->status = RUNNABLE;
		runq_add_thread(thr);
		waiting_thread = waiting_thread->next;
		del_entry(&thr->cond_wait_link);
	}
    mutex_unlock_int_save(&cv->queue_mutex);
}
