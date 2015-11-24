/** @file mutex.h
 *  @brief This file defines the interface for mutexes.
 */

#ifndef MUTEX_H
#define MUTEX_H

#include <sync/mutex_type.h>

void enable_mutex_lib();

int mutex_init( mutex_t *mp );
void mutex_destroy( mutex_t *mp );
void mutex_lock( mutex_t *mp );
void mutex_unlock( mutex_t *mp );
void mutex_lock_int_save( mutex_t *mp );
void mutex_unlock_int_save( mutex_t *mp );

#endif /* MUTEX_H */
