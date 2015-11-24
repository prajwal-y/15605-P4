/** @file task.c
 *  @brief task manipulation routines
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <common/malloc_wrappers.h>
#include <seg.h>
#include <cr.h>
#include <vm/vm.h>
#include <simics.h>
#include <stddef.h>
#include <eflags.h>
#include <core/task.h>
#include <asm/asm.h>
#include <core/thread.h>
#include <core/scheduler.h>
#include <loader/loader.h>
#include <ureg.h>
#include <syscall.h>
#include <string.h>
#include <common/errors.h>
#include <common/assert.h>

#define EFLAGS_RESERVED 0x00000002
#define EFLAGS_IOPL 0x00000000 
#define EFLAGS_IF 0x00000200 
#define EFLAGS_ALIGNMENT_CHECK 0xFFFbFFFF
	
static task_struct_t *init_task;
static task_struct_t *idle_task;
static uint32_t setup_user_eflags();
static void set_task_stack(void *kernel_stack_base, int entry_addr,
                           void *user_stack_top);
static void *copy_user_args(int num_args, char **argvec);
static void init_task_structures(task_struct_t *t);


/** @brief Create a new task
 *
 *  Creates a new task and adds the first thread to the task.
 *  The id of the first thread in the task is set to be the
 *  task ID. This function sets its parent to the parent struct passed
 *  in.
 *  
 *  @param parent the parent task of the new task
 *
 *  @return task_struct_t The reference to the new task
 *  NULL if task creation failed
 */
task_struct_t *create_task(task_struct_t *parent) {
	task_struct_t *t = (task_struct_t *)smalloc(sizeof(task_struct_t));
	if(t == NULL) {
		return NULL;
	}

    /* Set the parent of this task */
    t->parent = parent;

    /* Initialize various task data structures */
    init_task_structures(t);

    thread_struct_t *thr = create_thread(t);
	if(thr == NULL) {
		sfree(t, sizeof(task_struct_t));
		return NULL;
	}
	t->thr = thr;
	t->id = thr->id;
    return t;
}

/** @brief Function to initialize the task struct
 *
 *  @return void
 */
void init_task_structures(task_struct_t *t) {
    /* Initialize the thread list */
    init_head(&t->thread_head);

    /* Initialize the alive child task list */
    init_head(&t->child_task_head);

    /* Initialize the dead child task list */
    init_head(&t->dead_child_head);

    /* Initialize mutexes and cond_vars */
    mutex_init(&t->child_list_mutex);
    mutex_init(&t->thread_list_mutex);
    mutex_init(&t->vanish_mutex);
    cond_init(&t->exit_cond_var);           

    /* initialize swexn handler */
    t->eip = NULL;
    t->swexn_args = NULL;

	mutex_init(&t->fork_mutex);
	mutex_init(&t->exec_mutex);

}

/** @brief Creates a task for a given program and calls
 * the function to load the task. This function is only
 * called by init when the kernel boots
 *
 * @param prog_name Program for which the task has to be
 * created and loaded.
 *
 * @return void
 */
void load_init_task(char *prog_name) {
    /* Allocate memory for a task struct from kernel memory */
	task_struct_t *t = create_task(NULL);
    kernel_assert(t != NULL);

	char *args[] = {prog_name, 0};

    int retval = load_task(prog_name, 1, args, t);
    kernel_assert(retval == 0);
    init_task = t;
	
    runq_add_thread_interruptible(t->thr);
}

/** @brief start a bootstrap task
 *
 *  This process is started in a special way since it is one of
 *  the first processes to run on the system. We use an IRET
 *  to get to running this task.
 *
 *  @return void
 */
void load_bootstrap_task(char *prog_name) {

    int retval;
    /* ask vm to give us a zero filled frame for the page directory */
    void *pd_addr = create_page_directory();

    kernel_assert(pd_addr != NULL);

    /* Paging enabled! */
	set_cur_pd(pd_addr);

    /* Allocate memory for a task struct from kernel memory */
	task_struct_t *t = create_task(NULL);

    kernel_assert(t != NULL);
    t->pdbr = pd_addr;

    /* Read the idle task header to set up VM */
	simple_elf_t se_hdr;

    elf_load_helper(&se_hdr, prog_name);
    
    /* Invoke VM to setup the page directory/page table for a given binary */
    retval = setup_page_table(&se_hdr, pd_addr);
    kernel_assert(retval == 0);

    /* Copy program into memory */
    retval = load_program(&se_hdr);
    kernel_assert(retval == 0);

	set_running_thread(t->thr);
	t->thr->status = RUNNING;
    set_esp0(t->thr->k_stack_base);

	uint32_t EFLAGS = setup_user_eflags();
	unsigned long entry = se_hdr.e_entry;
	
	idle_task = t;

	enable_mutex_lib(); /* We need to enable mutex library */

	call_iret(EFLAGS, entry);
}

/** @brief Function to load a program into a given task.
 *
 *  @param prog_name Name of the program to be loaded
 *  @param num_args Number of arguments to the program
 *  @param argvec Character array of the arguments to the program
 *  @param t Reference to the task to which the program must be 
 *  		 loaded
 *
 *  @return 0 on success -ve integer on failure
 */
int load_task(char *prog_name, int num_args, char **argvec,
               task_struct_t *t) {

	int retval;
    /* ask vm to give us a zero filled frame for the page directory */
    void *pd_addr = create_page_directory();
    if (pd_addr == NULL) {
        return ERR_NOMEM;
    }

    /* Paging enabled! */
	set_cur_pd(pd_addr);

    t->pdbr = pd_addr;

    /* Read the idle task header to set up VM */
	simple_elf_t se_hdr;

    elf_load_helper(&se_hdr, prog_name);
    
    /* Invoke VM to setup the page directory/page table for a given binary */
    retval = setup_page_table(&se_hdr, pd_addr);
    if (retval < 0) {
        return retval;
    }

    /* Copy program into memory */
    retval = load_program(&se_hdr);
    if (retval < 0) {
        return retval;
    }

    /* Copy arguments onto user stack */
    void *user_stack_top = copy_user_args(num_args, argvec);
    if (user_stack_top == NULL) {
        return ERR_FAILURE;
    }

	set_task_stack((void *)t->thr->k_stack_base, 
					se_hdr.e_entry, user_stack_top);
	t->thr->cur_esp = (t->thr->k_stack_base - DEFAULT_STACK_OFFSET);

    return 0;
}

/* ------------ Static local functions --------------*/

/** @brief Function to hand create the stack for a new task to
 *          start executing
 *
 *  @param kernel_stack_base Stack high for the kernel stack of the task
 *  @param entry_addr Entry point for the program
 *  @param user_stack_top The stack low for the program
 *
 *  @return void
 */
void set_task_stack(void *kernel_stack_base, int entry_addr, 
                    void *user_stack_top) {
    /* Hand craft the kernel stack for these tasks */
    /* Add the registers required for IRET */
	uint32_t EFLAGS = setup_user_eflags();

    *((int *)(kernel_stack_base) - DS_OFFSET) = SEGSEL_USER_DS;
    *((int *)(kernel_stack_base) - STACK_OFFSET) = (int)user_stack_top;
    *((int *)(kernel_stack_base) - EFLAGS_OFFSET) = EFLAGS;
    *((int *)(kernel_stack_base) - CS_OFFSET) = SEGSEL_USER_CS;
    *((int *)(kernel_stack_base) - EIP_OFFSET) = entry_addr;

    /* Simulate a pusha for the 8 registers that are pushed */
    memset(((int *)(kernel_stack_base) - PUSHA_OFFSET), 0, PUSHA_SIZE);

	*((int *)(kernel_stack_base) - IRET_FUN_OFFSET) = (int)iret_fun;
}

/** @brief Function to copy the arguments to a given program
 *  on to the user space task stack
 *
 *  @param num_args Number of arguments to the program
 *  @param argvec Character array of the argument vector
 *
 *  @return void* Address of the top of user stack
 */
void *copy_user_args(int num_args, char **argvec) {
    char *user_stack_top = (char *)STACK_START;
    char **argvec_copy = (char **)smalloc((num_args + 1) * sizeof(char *));
    if (argvec_copy == NULL) {
        return NULL;
    }
    int i;
    for (i = 0; i < num_args; i++) {
        user_stack_top -= strlen(argvec[i]) + 1;
        strcpy((void *)user_stack_top, argvec[i]);
        argvec_copy[i] = user_stack_top;
    }
    user_stack_top -= 1;
    *user_stack_top ='\0';
    argvec_copy[i] = (char *)user_stack_top;
    user_stack_top -= (num_args + 1) * sizeof(char *);
    memcpy(user_stack_top, (void *)argvec_copy, (num_args + 1) * sizeof(char *));
    char **argvec_usr = (char **)user_stack_top;
    user_stack_top -= sizeof(int *);
    *(int *)user_stack_top = STACK_END;
    user_stack_top -= sizeof(int *);
    *(int *)user_stack_top = STACK_START;
    user_stack_top -= sizeof(char **);
    *(int *)user_stack_top = (int)argvec_usr;
    user_stack_top -= sizeof(int *);
    *user_stack_top = (int)num_args;
    user_stack_top -= sizeof(int *);
    *user_stack_top = (int)0;
    sfree(argvec_copy, (num_args + 1) * sizeof(char *));

    return user_stack_top;
}

/** @brief set user EFLAGS 
 *
 *  The EFLAGS for user have interrupts enabled (IF), IOPL set to 3
 *  (privilege level 3), alignment check off and bit 1 set.
 *
 *  @return void
 */
uint32_t setup_user_eflags() {
	uint32_t eflags = get_eflags();
    eflags |= EFLAGS_RESERVED;          /*Reserved bit*/
	eflags |= EFLAGS_IOPL;              /*IOPL*/
	eflags |= EFLAGS_IF;                /*IF*/
	eflags &= EFLAGS_ALIGNMENT_CHECK;   /*Alignment check off*/
	return eflags;
}

/** @brief get a pointer to the init task
 *
 * @return task_struct_t* pointer to the init task
 */
task_struct_t *get_init_task() {
    return init_task;
}

/** @brief get a pointer to the idle task
 *
 * @return task_struct_t* pointer to the idle task
 */
task_struct_t *get_idle_task() {
    return idle_task;
}
