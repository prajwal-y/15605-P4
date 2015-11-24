/** @file wait_vanish.c
 *
 *  File which implements the functions required for the wait
 *  and vanish system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <asm.h>
#include <core/fork.h>
#include <core/scheduler.h>
#include <common/errors.h>
#include <core/thread.h>
#include <core/context.h>
#include <common/malloc_wrappers.h>
#include <asm/asm.h>
#include <vm/vm.h>
#include <simics.h>
#include <ureg.h>
#include <syscalls/syscall_util.h>

#define ALIVE_TASK 0
#define DEAD_TASK 1

static void remove_thread_from_task(thread_struct_t *thr);
static void thread_free_resources(thread_struct_t *thr);
static void reparent_to_init(list_head *task_list, int task_type, 
                      task_struct_t *init_task);

/** @brief The entry point for wait system call
 *
 *  @param arg_packet The address of argument packet containing 
 *  the required arguments for wait.
 *
 *  @return The task ID (id of the first thread in the task) if wait returns.
 *          -1 if wait cannot wait on any task exiting in the future or if 
 *          status_ptr is invalid
 */
int do_wait(void *arg_packet) {

    int *status_ptr = (int *)arg_packet;
    if (status_ptr != NULL && 
			((is_pointer_valid(status_ptr, sizeof(int *)) < 0)
        		|| (is_memory_writable(status_ptr, sizeof(int *)) < 0))) {
        return ERR_INVAL;
    }
    
    task_struct_t *curr_task = get_curr_task();
    thread_struct_t *curr_thread = get_curr_thread();

    mutex_lock(&curr_task->child_list_mutex);
    list_head *dead_head = get_first(&curr_task->dead_child_head);
    list_head *alive_head = get_first(&curr_task->child_task_head);
			
    /* We will block wait if there are no dead tasks and there are still some 
     * tasks alive */
    while (dead_head == NULL && alive_head != NULL) {
        cond_wait(&curr_task->exit_cond_var, &curr_task->child_list_mutex,
                  &curr_thread->cond_wait_link, WAITING);
        dead_head = get_first(&curr_task->dead_child_head);
        alive_head = get_first(&curr_task->child_task_head);
    }

    /* If there is something in the dead task list */
    if (dead_head != NULL) {
        del_entry(dead_head);
        mutex_unlock(&curr_task->child_list_mutex);

        task_struct_t *dead_task = get_entry(dead_head, task_struct_t,
                                             dead_child_link);
        int dead_task_id = dead_task->id;
        if (status_ptr != NULL) {
            *status_ptr = dead_task->exit_status;
        }
	
		/* Free task resources */
		mutex_destroy(&dead_task->child_list_mutex);
		mutex_destroy(&dead_task->thread_list_mutex);
		mutex_destroy(&dead_task->vanish_mutex);
		mutex_destroy(&dead_task->fork_mutex);
		mutex_destroy(&dead_task->exec_mutex);
		cond_destroy(&dead_task->exit_cond_var);
        sfree(dead_task, sizeof(task_struct_t));
        return dead_task_id;
    }
    mutex_unlock(&curr_task->child_list_mutex);
    return ERR_FAILURE;
}


void do_vanish() {
    task_struct_t *curr_task = get_curr_task();
	thread_struct_t *curr_thread = get_curr_thread();
    task_struct_t *init_task = get_init_task();

    mutex_lock(&curr_task->thread_list_mutex);
	remove_thread_from_task(curr_thread);
    list_head *thread_head = get_first(&curr_task->thread_head);
    mutex_unlock(&curr_task->thread_list_mutex);
    /* If this is the last thread in the task, take care 
 	 * of dead and alive tasks in the current task */
    if (thread_head == NULL) {
    
        /* We need to disable interrupts when re-parenting the child tasks. 
 		  	This is a little inefficient. But using a mutex on the parent
			task can cause memory corruption problems as parent can exit
			before child unlocks the mutex. */
        disable_interrupts();
        mutex_lock(&curr_task->vanish_mutex);

        /* Make parent of dead and alive tasks to init */
        reparent_to_init(&curr_task->child_task_head, ALIVE_TASK, init_task);
        reparent_to_init(&curr_task->dead_child_head, DEAD_TASK, init_task);

        mutex_lock(&init_task->child_list_mutex);
        concat_lists(&init_task->child_task_head, &curr_task->child_task_head);
        concat_lists(&init_task->dead_child_head, &curr_task->dead_child_head);
        mutex_unlock(&init_task->child_list_mutex);

        mutex_unlock(&curr_task->vanish_mutex);
        enable_interrupts();
		
		void *curr_pdbr = curr_task->pdbr;
		curr_task->pdbr = get_kernel_pd();
        set_kernel_pd();
       	free_paging_info(curr_pdbr);

		disable_interrupts(); /* Ensuring that only I run after signaling the parent */
	    task_struct_t *parent_task = curr_task->parent;	
        del_entry(&curr_task->child_task_link);
        add_to_tail(&curr_task->dead_child_link, &parent_task->dead_child_head);
        list_head *parent_alive_head = get_first(&parent_task->child_task_head);
        if (parent_alive_head == NULL) {
            cond_broadcast(&parent_task->exit_cond_var);
        }
        else {
            cond_signal(&parent_task->exit_cond_var);
        }
    }
	/* Time to free the thread. So we use the special kernel stack. */
	/* We need to disable interrupts so that only one thread uses the special stack */
	disable_interrupts();
	void *dead_thr_kernel_stack = get_dead_thr_kernel_stack();
	update_to_dead_thr_stack((uint32_t)dead_thr_kernel_stack);
	thread_struct_t *dead_thread = get_curr_thread();
    thread_free_resources(dead_thread);
	set_running_thread(NULL);
    context_switch();
}

/** @brief Function to remove a thread from its parent
 *  task
 *
 *  @param thr Thread which must be removed from its 
 *  		parent's list of threads
 *
 *  @return void
 */
void remove_thread_from_task(thread_struct_t *thr) {
    remove_thread_from_map(thr->id);
    del_entry(&thr->task_thread_link);
}

/** @brief Free the resources associated with a thread
 *
 *  Free the thread kernel stack and the thread struct
 *  itself.
 *
 *  @param thr the thread who will go missing soon
 *  @return void
 */ 
void thread_free_resources(thread_struct_t *thr) {
	mutex_destroy(&thr->deschedule_mutex);
	cond_destroy(&thr->deschedule_cond_var);
	sfree(thr, sizeof(thread_struct_t));
}

/** @brief reparent a list of tasks to init
 *
 *  @param list_head the head of the task list (alive/dead)
 *  @param task_type whether we are rparenting a list of dead tasks
 *         or alive tasks
 *  @return void
 */
void reparent_to_init(list_head *task_list, int task_type, 
                      task_struct_t *init_task) {
    list_head *task_node = get_first(task_list);
    while(task_node != NULL && task_node != task_list) {
        task_struct_t *task;
        if (task_type == ALIVE_TASK) {
            task = get_entry(task_node, task_struct_t, child_task_link);
        }
        else {
            task = get_entry(task_node, task_struct_t, dead_child_link);
        }
        task->parent = init_task; 
        task_node = task_node->next;
    }
}
