/** @file fork.c
 *
 *  File which implements the functions required for fork
 *  system call.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <core/task.h>
#include <core/thread.h>
#include <core/scheduler.h>
#include <vm/vm.h>
#include <asm/asm.h>
#include <common/errors.h>
#include <string.h>
#include <syscall.h>
#include <simics.h>
#include <common/malloc_wrappers.h>

static void thread_free_resources(thread_struct_t *thr);

/** @brief The entry point for fork
 *
 *  @return int the ID of the new task. If fork
 *  fails, then a negative number is returned.
 */
int do_fork() {
	task_struct_t *curr_task = get_curr_task();

	/* Allow only one thread per task to fork() */
	mutex_lock(&curr_task->fork_mutex);

	/* Create a child task */
	task_struct_t *child_task = create_task(curr_task);
	if(child_task == NULL) {
		mutex_unlock(&curr_task->fork_mutex);
		return ERR_NOMEM;
	}
    mutex_lock(&curr_task->vanish_mutex);
    add_to_tail(&child_task->child_task_link, &curr_task->child_task_head);
    mutex_unlock(&curr_task->vanish_mutex);
	
	/* Clone the address space */
	void *new_pd_addr = clone_paging_info(curr_task->pdbr);
	if(new_pd_addr == NULL) {
		thread_free_resources(child_task->thr);
		sfree(child_task, sizeof(task_struct_t));
		mutex_unlock(&curr_task->fork_mutex);
		return ERR_FAILURE;
	}
	child_task->pdbr = new_pd_addr;

	/* Copy the software exception handler data */
	child_task->eip = curr_task->eip;
	child_task->swexn_args = curr_task->swexn_args;
	child_task->swexn_esp = curr_task->swexn_esp;

	/* Clone the kernel stack */
	memcpy(child_task->thr->k_stack, curr_task->thr->k_stack, 
							KERNEL_STACK_SIZE);
	*((int *)(child_task->thr->k_stack_base) - 
							IRET_FUN_OFFSET) = (int)iret_fun;
	child_task->thr->cur_esp = child_task->thr->k_stack_base 
									- DEFAULT_STACK_OFFSET;

	/* Add the first thread of the new task to runnable queue */
	runq_add_thread(child_task->thr);

	/* Dummy operation to invalidate TLB */
	set_cur_pd(curr_task->pdbr);

	mutex_unlock(&curr_task->fork_mutex);

	return child_task->id;	
}

/** @brief Entry point for thread fork
 *
 *  This function creates a new thread for the current task and
 *  adds the thread to the runnable queue.
 *
 *  @return the thread ID of the new thread
 */
int do_thread_fork() {
	task_struct_t *curr_task = get_curr_task();

	thread_struct_t *curr_thread = get_curr_thread();
	thread_struct_t *child_thread = create_thread(curr_task);
	if(child_thread == NULL) {
		return ERR_FAILURE;
	}

	/* Clone the kernel stack */
	memcpy(child_thread->k_stack, curr_thread->k_stack, 
								KERNEL_STACK_SIZE);
	*((int *)(child_thread->k_stack_base) - 
								IRET_FUN_OFFSET) = (int)iret_fun;
	child_thread->cur_esp = child_thread->k_stack_base 
								- DEFAULT_STACK_OFFSET;

	/* Add the first thread of the new task to runnable queue */
	runq_add_thread(child_thread);

	return child_thread->id;
}

/** @brief Free the resources associated with a thread
 *  
 *  Free the thread kernel stack and the thread struct
 *  itself.
 *
 *  @param thr the thread who will go missing soon
 *  @return void
 **/
void thread_free_resources(thread_struct_t *thr) {
	mutex_destroy(&thr->deschedule_mutex);
	cond_destroy(&thr->deschedule_cond_var);
    sfree(thr, sizeof(thread_struct_t));
}
