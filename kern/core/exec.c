/** @file exec.c
 *
 *  File which implements the functions required for the exec
 *  system call.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <simics.h>
#include <malloc.h>
#include <elf_410.h>
#include <common/assert.h>
#include <string/string.h>
#include <vm/vm.h>
#include <cr.h>
#include <common/errors.h>
#include <core/task.h>
#include <core/task.h>
#include <core/scheduler.h>
#include <loader/loader.h>
#include <syscalls/syscall_util.h>

static int get_num_args(char **argvec);
static char **copy_args(int num_args,char **argvec);
static void free_args(char **argvec, int num);

/** @brief The entry point for exec
 *
 *  @param arg_packet The address of argument packet containing 
 *  the required arguments for exec.
 *
 *  @return nothing if the call succeeds. If exec
 *  fails, then a negative number is returned.
 */
int do_exec(void *arg_packet) {
    task_struct_t *t = get_curr_task();

    /* Prevent multiple threads from same task from running exec at the 
     * same time */
    mutex_lock(&t->exec_mutex);

    char *execname = (char *)(*((int *)arg_packet));
    char **argvec = (char **)(*((int *)arg_packet + 1));
    int num_args, retval;

    /* Copy execname to kernel memory after checking validity */
    char execname_kern[EXECNAME_MAX];
    if (execname_kern == NULL) {
    	mutex_unlock(&t->exec_mutex);
        return ERR_NOMEM;
    }
    if (copy_user_data(execname_kern, execname, EXECNAME_MAX) < 0) {
    	mutex_unlock(&t->exec_mutex);
        return ERR_INVAL;
    }

    /* Check if program exists in ramdisk and is a valid ELF prog */
    retval = check_program(execname_kern);
    if (retval == PROG_ABSENT_INVALID) {
    	mutex_unlock(&t->exec_mutex);
        return ERR_FAILURE;
    }

    /* Count number of arguments in argvec returning an error if exceeding
     * the maximum number of arguments */
    num_args = get_num_args(argvec);
    if (num_args < 0) {
    	mutex_unlock(&t->exec_mutex);
        return ERR_FAILURE;
    }
    void *old_pd = (void *)get_cr3();

    /* Copy the argument vector into kernel space as we will be freeing
     * the old process's address space soon */
    char **argvec_kern = copy_args(num_args, argvec);
    if (argvec_kern == NULL) {
    	mutex_unlock(&t->exec_mutex);
        return ERR_FAILURE;
    }

    retval = load_task(execname_kern, num_args, argvec_kern, t);
    if (retval < 0) {
        free_args(argvec_kern, num_args);
        set_cur_pd(old_pd);
    	mutex_unlock(&t->exec_mutex);
        return retval;
    }

    /* Free kernel argvec and execname */
    free_paging_info(old_pd);
    free_args(argvec_kern, num_args);

    mutex_unlock(&t->exec_mutex);

    return 0;
}

/** @brief Function to copy the arguments to kernel memory
 *
 *  @param num_args Number of arguments
 *  @param argvec Character array of arguments
 *
 *  @return char** copied character vector in kernel memory.
 *  NULL on failure.
 */
char **copy_args(int num_args,char **argvec) {
    int i;
    char *arg;
    char **argvec_kern = (char **)smalloc((num_args + 1) * sizeof(char *));
    if(argvec_kern == NULL) {
        return NULL;
    }
    for (i = 0; i < num_args; i++) {
        arg = (char *)smalloc(ARGNAME_MAX);
        if (arg == NULL) {
            free_args(argvec_kern, i);
            return NULL;
        }
        if (copy_user_data(arg, argvec[i], ARGNAME_MAX) < 0) {
            free_args(argvec_kern, i);
            return NULL;
        }
        argvec_kern[i] = arg;
    }
    arg = (char *)smalloc(sizeof(char));
    if(arg == NULL) {
        free_args(argvec_kern, num_args);
        return NULL;
    }
    arg[0] = '\0';
    argvec_kern[i] = arg;
    return argvec_kern;
}

/** @brief free the kernel args
 *
 *  @param argvec the argument array to be freed
 *  @param num_args number of arguments
 *
 *  @return void
 */
void free_args(char **argvec, int num_args) {
    int i;
    for (i = 0; i < num_args; i++) {
        sfree(argvec[i], ARGNAME_MAX);
    }
    sfree(argvec[i], sizeof(char));
    sfree(argvec, (num_args + 1) * sizeof(char *));
}


/** @brief Function to get the number of arguments from
 *         the argument vector
 *
 *  The argvec pointer is in user space and as such we need to validate before
 *  accessing
 *
 *  @param argvec The argument vector in user memory
 *  @return int The number of arguments or ERR_BIG if more than the 
 *              max allowed number of arguments
 */
int get_num_args(char **argvec) {
    int count = 0;
    if (is_pointer_valid(argvec, sizeof(char *)) < 0) {
        return ERR_INVAL;
    }

    while ((is_pointer_valid(argvec[count], sizeof(char *)) == 0)
            && argvec[count] != '\0') {
        count++;
        if (count > NUM_ARGS_MAX) {
            return ERR_BIG;
        }
    }
    return count;
}
