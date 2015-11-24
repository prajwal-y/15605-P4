/** @file thread.h
 *  @brief prototypes of thread manipulation routines
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __THREAD_H
#define __THREAD_H

#include <ureg.h>
#include <stdint.h>
#include <list/list.h>
#include <core/task.h>
#include <syscall.h>
#include <sync/mutex.h>
#include <sync/cond_var.h>

#define KERNEL_STACK_SIZE ((PAGE_SIZE) * 3)
/* Thread states */
#define RUNNING 0
#define RUNNABLE 1
#define WAITING 2
#define EXITED 3
#define DESCHEDULED 4

/** @brief a schedulable "unit"
 *
 *  Tasks contain threads. A thread contains all the context (registers) 
 *  required to be scheduled by the processor. A task does not contain any
 *  scheduling information. A task must contain atleast one thread.
 */
typedef struct thread_struct {
    int id;                     /* A unique identifier for a thread */
    task_struct_t *parent_task; /* The parent task for this thread */
    char k_stack[KERNEL_STACK_SIZE];	/* Kernel stack for the thread */
	uint32_t k_stack_base;		/* Top of the kernel stack for the thread */
	uint32_t cur_esp;		 	/* Current value of the kernel stack %esp */
	uint32_t cur_ebp;			/* Current value of the kernel stack %ebp */
	int status;         	    /* Life state of the thread */
    list_head runq_link;        /* Link structure for the run queue */
    list_head sleepq_link;      /* Link structure for the sleep queue */
    list_head thread_map_link;  /* Link structure for the hash map */
	list_head cond_wait_link;	/* Link structure for cond_wait */
	list_head mutex_link;		/* Link structure for mutex */
    list_head task_thread_link; /* Link structure for list of threads in parent */
    long wake_time;             /* Time when this thread is to be woken up */

    /* Mutex to protect use of the "reject" variable while descheduling */
    mutex_t deschedule_mutex;  
    /* Condition variable on which a thread waits if descheduled */ 
    cond_t deschedule_cond_var;
} thread_struct_t;

void kernel_threads_init();

thread_struct_t *create_thread(task_struct_t *task);

thread_struct_t *get_thread_from_id(int thr_id);

void remove_thread_from_map(int thr_id);

#endif  /* __THREAD_H */
