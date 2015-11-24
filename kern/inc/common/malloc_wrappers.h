/** @file malloc_wrappers.h
 *  @brief thread safe versions of malloc functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __MALLOC_WRAPPERS_H
#define __MALLOC_WRAPPERS_H

#include<stddef.h>

void init_thr_safe_malloc_lib();

void *malloc(size_t size);
void *memalign(size_t alignment, size_t size);
void *realloc(void *buf, size_t new_size);
void free(void *buf);
void *smalloc(size_t size);
void *smemalign(size_t alignment, size_t size);
void sfree(void *buf, size_t size);

#endif  /* __MALLOC_WRAPPERS_H */
