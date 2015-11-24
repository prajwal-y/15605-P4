/** @file scheduler.c
 *  @brief implementation of scheduler functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <asm.h>
#include <core/thread.h>
#include <list/list.h>
#include <core/scheduler.h>
#include <sync/mutex.h>
#include <simics.h>
#include <core/sleep.h>
#include <drivers/timer/timer.h>

static thread_struct_t *curr_thread; /* The thread currently being run */

static list_head runnable_threads;    /* List of runnable threads */

static thread_struct_t *runq_get_head();

/** @brief initialize the scheduler data structures
 *
 *  @return void
 */
void init_scheduler() {
	init_head(&runnable_threads);
    init_sleeping_threads();
}

/** @brief return the next thread to be run
 *
 *  Implements a round robin scheduling strategy. Returns the next thread
 *  to be run. This will be invoked by context switching code ONLY.
 *
 *  @return thread_struct_t a struct containing scheduling information 
 *                          for the next thread
 */
thread_struct_t *next_thread() {
    /* See if any sleeping thread can be woken up */
    thread_struct_t *sleeping_thread = get_sleeping_thread();
    if (sleeping_thread != NULL) {
        return sleeping_thread;
    }

    /* Get the thread at the head of the runqueue */
    thread_struct_t *head = runq_get_head();
    if (head == NULL) {
        return NULL;
    }
    return head;
}

/** @brief Function to get the first thread present in the runnable queue.
 *
 *  @return thread_struct_t * Pointer to the thread struct.
 */
thread_struct_t *runq_get_head() {
    list_head *head = get_first(&runnable_threads);
    if (head == NULL) {
        return NULL;
    }
    thread_struct_t *head_thread = get_entry(head, thread_struct_t, runq_link);
    del_entry(head);
    return head_thread;
}

/** @brief Function to add a particular thread to the runnable queue.
 *
 *  This function disable interrupts when adding the thread to the queue
 *  to maintain the data structure consistency.
 *
 *  @param thr The thread struct that must be added to the runnable queue
 *
 *  @return void
 */
void runq_add_thread(thread_struct_t *thr) {
    disable_interrupts();
    add_to_tail(&thr->runq_link, &runnable_threads);
    enable_interrupts();
}

/** @brief Function to add a particular thread to the runnable queue.
 *
 *  This function is called only from places where interrupts are
 *  disabled. Example, from context_switch()
 *
 *  @param thr The thread struct that must be added to the runnable queue
 *
 *  @return void
 */
void runq_add_thread_interruptible(thread_struct_t *thr) {
    add_to_tail(&thr->runq_link, &runnable_threads);
}

/** @brief get the currently running thread
 *
 *  @return thread_struct_t thread info of the currently running thread
 */
thread_struct_t *get_curr_thread() {
    return curr_thread;
}

/** @brief get the currently running task
 *
 *  @return task_struct_t Task info of the current task
 */
task_struct_t *get_curr_task() {
	return curr_thread->parent_task;
}

/** @brief Set the currently running thread
 *
 *  @param thr The thread struct for the thread to be set
 *  as the currently running thread
 *
 *  @return void
 */
void set_running_thread(thread_struct_t *thr) {
    curr_thread = thr;
}

/** @brief Prints the runnable thread list
 *
 *  Used for debugging
 *
 *  @return void
 */
void print_runnable_list() {
	list_head *temp = get_first(&runnable_threads);
	lprintf("-------Beginning of runnable threads--------");
	while(temp != NULL && temp != &runnable_threads) {
		thread_struct_t *thr = get_entry(temp, thread_struct_t, runq_link);
		lprintf("-------Thread %d-------", thr->id);
		temp = temp->next;
	}
	lprintf("--------End of runnable threads-------");
}
