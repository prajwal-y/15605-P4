/** @file thread_syscalls.c
 *
 *  @brief implementation of functions for thread management
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <syscalls/thread_syscalls.h>
#include <core/thread.h>
#include <core/sleep.h>
#include <core/scheduler.h>
#include <core/context.h>
#include <common/errors.h>
#include <simics.h>
#include <drivers/timer/timer.h>
#include <syscall.h>
#include <syscalls/syscall_util.h>
#include <ureg.h>
#include <vm/vm.h>

/** @brief implement the functionality to get the tid
 *         from the global curr_thread struct. This passes
 *         the value back to the assembly wrapper which performs
 *         an IRET to return the current thread id to the user
 *
 *  @return int the thread id of the current thread
 */
int gettid_handler_c() {
    thread_struct_t *curr_thread = get_curr_thread();
    return curr_thread->id;
}

/** @brief yield to a different thread
 *
 *  @return int 0 on success -ve integer if tid does not exist or
 *              thread is suspended
 */
int yield_handler_c(int tid) {
    if (tid != -1) {
        thread_struct_t *thr = get_thread_from_id(tid);
        if (thr == NULL) {
            return ERR_INVAL;
        }
        if (thr->status == WAITING || thr->status == DESCHEDULED) {
            return ERR_FAILURE;
        }
    }
    context_switch();
    return 0;
}

/** @brief sleep for a specified amount of time
 *
 *  @return int 0 on success -ve integer if ticks is negative
 */
int sleep_handler_c(int ticks) {
    return do_sleep(ticks);
}

/** @brief deschedule a thread
 *
 *  @return int 0 immediately if integer pointed by reject is non zero
 *              0 on make_runnable if value at reject is 0. -ve integer
 *              if reject is invalid pointer.
 */
int deschedule_handler_c(int *reject) {
    if (is_pointer_valid(reject, sizeof(int *)) < 0) {
        return ERR_INVAL;
    }
    thread_struct_t *thr = get_curr_thread();
    mutex_lock(&thr->deschedule_mutex);
    if (*reject != 0) {
        mutex_unlock(&thr->deschedule_mutex);
        return 0;
    }
    cond_wait(&thr->deschedule_cond_var, &thr->deschedule_mutex, 
              &thr->cond_wait_link, DESCHEDULED);
    mutex_unlock(&thr->deschedule_mutex);
    return 0;
}

/** @brief make_runnable a thread
 *
 *  @param tid the thread id that must be made runnable
 *  @return int 0 on success, -ve integer if the thread is not currently
 *              DESCHEDULE'd
 */
int make_runnable_handler_c(int tid) {
    if (tid < 0) {
        return ERR_INVAL;
    }
    thread_struct_t *thr = get_thread_from_id(tid);
    if (thr == NULL) {
        return ERR_INVAL;
    }
    mutex_lock(&thr->deschedule_mutex);
    if (thr->status != DESCHEDULED) {
        mutex_unlock(&thr->deschedule_mutex);
        return ERR_INVAL;
    }
    cond_signal(&thr->deschedule_cond_var);
    mutex_unlock(&thr->deschedule_mutex);
    return 0;
}

/** @brief get the number of ticks since system boot
 *
 *  @return unsigned int number of ticks since system boot
 */
unsigned int get_ticks_handler_c() {
    return total_ticks();
}

/** @brief install a swexn handler
 *
 *  @param esp3 the exception stack
 *  @param eip points to the first instruction of exception handler
 *  @param arg arguemnt to the swexn handler
 *  @param newureg the new register values to take when returning
 *
 *  @return int 0 on success, -ve integer on failure
 */
int swexn_handler_c(void *arg_packet) {
    void *esp3 = (void *)(*((int *)arg_packet));
    if (esp3 != NULL && (is_pointer_valid(esp3, 4) < 0
        || is_memory_writable(esp3, 4) < 0)) {
        return ERR_INVAL;
    }

    swexn_handler_t eip = (void *)(*((int *)arg_packet + 1));
    if (eip != NULL && (is_pointer_valid(eip, 4) < 0)) {
        return ERR_INVAL;
    }

    void *arg = (void *)(*((int *)arg_packet + 2));

    ureg_t *newureg = (ureg_t *)(*((int *)arg_packet + 3));
    if (newureg != NULL && (is_pointer_valid(newureg, 4) < 0)) {
        return ERR_INVAL;
    }

    task_struct_t *curr_task = get_curr_task();
    thread_struct_t *curr_thread = get_curr_thread();
    if (esp3 == NULL || eip == NULL) {
        curr_task->eip = NULL;
		return 0;
    }

    curr_task->eip = eip;
    curr_task->swexn_args = arg;
    curr_task->swexn_esp = esp3;

    if (newureg != NULL) {
        int retval = setup_kernel_stack(newureg, 
                                       (void *)curr_thread->k_stack_base);
        if (retval < 0) {
            return ERR_FAILURE;
        }
		return newureg->eax;
    }
    return 0;
}
