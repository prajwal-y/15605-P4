/** @file sem.h
 *  @brief This file defines the interface to semaphores
 */

#ifndef __SEM_H
#define __SEM_H

#include <sync/sem_type.h>

/* semaphore functions */
int sem_init(sem_t *sem, int count);
void sem_wait(sem_t *sem);
void sem_signal(sem_t *sem);
void sem_destroy(sem_t *sem);

#endif /* __SEM_H */
