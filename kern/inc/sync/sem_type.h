/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef __SEM_TYPE_H
#define __SEM_TYPE_H

#include <sync/cond_var.h>
#include <sync/mutex.h>

typedef struct sem {
	mutex_t mutex;
	cond_t cond_var;
	int count;
	int valid;
} sem_t;

#endif /* __SEM_TYPE_H */
