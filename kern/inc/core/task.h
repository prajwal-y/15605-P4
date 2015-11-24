/** @file task.h
 *  @brief task manipulation routines
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __TASK_H
#define __TASK_H

#include <list/list.h>
#include <core/task.h>
#include <sync/cond_var.h>
#include <sync/mutex.h>
#include <sync/sem.h>
#include <syscall.h>

#define DEFAULT_STACK_OFFSET 56 

#define DS_OFFSET 1
#define STACK_OFFSET 2
#define EFLAGS_OFFSET 3
#define CS_OFFSET 4
#define EIP_OFFSET 5
#define PUSHA_OFFSET 13
#define PUSHA_SIZE 32
#define IRET_FUN_OFFSET 14

#define EXECNAME_MAX 255
#define NUM_ARGS_MAX 16
#define ARGNAME_MAX 255 


struct thread_struct;

/** @brief the protection domain comprising a task */
typedef struct task_struct {
	int id;
    void *pdbr;         	        /* the address of the page directory */
	struct thread_struct *thr;	    /* the reference to the first thread */
    list_head thread_head;          /* List of threads in the task */
    int exit_status;                /* Exit status of task */
    struct task_struct *parent;     /* Link to the parent task */

    list_head child_task_head;      /* Linked list head for alive child tasks of THIS task */
    list_head child_task_link;      /* Link for list of alive child tasks in parent */ 
    
    list_head dead_child_head;      /* Linked list head for dead children of THIS task */
    list_head dead_child_link;      /* Link for list of dead children in parent */

    swexn_handler_t eip;            /* The swexn handler function */
    void *swexn_args;               /* Arguments to the swexn function */
    void *swexn_esp;                /* ESP to run the swexn handler on */

    /* Cond var for threads of THIS task to wait on child vanish()es */
    cond_t exit_cond_var;           
    /* Mutex to synchronize access to dead and alive child task lists */    
    mutex_t child_list_mutex;             
    /* Mutex to synchronize access to the current task's thread list */    
    mutex_t thread_list_mutex;
    /* Mutex to synchronize access to the vanish() system call */    
    mutex_t vanish_mutex;
	
	/* Used when tasks containing multiple threads call system calls */
	mutex_t fork_mutex;	/* Semaphore to allow only one fork to run per task */
	mutex_t exec_mutex; /* Semaphore to allow only one exec to run per task */
     
} task_struct_t;

task_struct_t *create_task(task_struct_t *parent);

void load_bootstrap_task(char *prog_name);

void load_init_task(char *prog_name);

int load_task(char *prog_name, int num_arg, char **argvec, 
               task_struct_t *t);

task_struct_t *get_init_task();

task_struct_t *get_idle_task();

#endif  /* __TASK_H */

