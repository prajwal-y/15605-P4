/** @file sem.c
 *  @brief Implementation of semaphores
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <sync/cond_var.h>
#include <sync/mutex.h>
#include <sync/sem.h>
#include <list/list.h>
#include <common/assert.h>
#include <simics.h>
#include <core/scheduler.h>
#include <core/thread.h>

/** @brief This function will initialize a semaphore to
 * 	a given value count. 
 *
 *  @param sem Semaphore to be initialized
 *  @param count Count of the semaphore
 *  @return int 0 on success, error code on failure
 */
int sem_init(sem_t *sem, int count) {
	if(sem == NULL) {
		return -1;
	}
	mutex_init(&sem->mutex);
	cond_init(&sem->cond_var);
	sem->count = count;
	sem->valid = 1;
	return 0;
}

/**
 * @brief This function allows the semaphore to be decremented.
 * The function blocks until the count of the semaphore is
 * valid to be decremented.
 *
 * @param sem Semaphore whose count should be decremented.
 * @return Void
 */
void sem_wait(sem_t *sem) {
	thread_assert(sem != NULL);
	thread_assert(sem->valid);
    mutex_lock(&sem->mutex);
	thread_struct_t *curr_thread = get_curr_thread();
	while(sem->count == 0) {
		cond_wait(&sem->cond_var, &sem->mutex, 
					&curr_thread->cond_wait_link, WAITING);
	}
	sem->count--;
    mutex_unlock(&sem->mutex);
}

/**
 * @brief Function to wake up a thread waiting on the 
 * semaphore. The value of the count of the semaphore
 * is incremented before signaling the waiting threads.
 *
 * @param sem Semaphore whose count needs to be increased
 * @return Void
 */
void sem_signal(sem_t *sem) {
	thread_assert(sem != NULL);
	thread_assert(sem->valid);
    mutex_lock(&sem->mutex);
	sem->count++;
	cond_signal(&sem->cond_var);
    mutex_unlock(&sem->mutex);
}

/**
 * @brief Function to deactivate a semaphore.
 *
 * @param sem Semaphore which needs to be deactivated
 * @return Void
 */
void sem_destroy(sem_t *sem) {
	thread_assert(sem != NULL);
	cond_destroy(&sem->cond_var);
	mutex_destroy(&sem->mutex);
    sem->valid = 0;
}
