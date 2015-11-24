/** @file cond.h
 *  @brief This file defines the interface for condition variables.
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef __COND_H
#define __COND_H

#include <sync/mutex.h>
#include <sync/cond_type.h>
#include <list/list.h>

/* condition variable functions */
int cond_init(cond_t *cv);
void cond_destroy(cond_t *cv);
void cond_wait(cond_t *cv, mutex_t *mp, list_head *link, 
               int status);
void cond_signal(cond_t *cv);
void cond_broadcast(cond_t *cv);

#endif /* COND_H */
