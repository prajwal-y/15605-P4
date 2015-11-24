/** @file vm.h
 *  @brief prototypes for virtual memory
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __VM_H
#define __VM_H

#include <elf_410.h>

#define PAGE_ENTRY_PRESENT 1
#define READ_WRITE_ENABLE 2
#define WRITE_DISABLE_MASK 0xFFFFFFFD
#define USER_MODE 4
#define COW_MODE 512
#define COW_MODE_DISABLE_MASK 0xFFFFFDFF
#define WRITE_THROUGH_CACHING 8
#define DISABLE_CACHING 16
#define GLOBAL_PAGE_ENTRY 256
#define NEWPAGE_PAGE 1024
#define NEWPAGE_START 2048
#define NEWPAGE_END 3072

/*Constants and macros*/

#define PAGE_TABLE_ENTRY_SIZE 4
#define PAGE_DIRECTORY_MASK 0xffc00000
#define PAGE_TABLE_MASK 0x003ff000
#define PAGE_ROUND_DOWN 0xfffff000
#define NUM_PAGE_TABLE_ENTRIES (PAGE_SIZE / PAGE_TABLE_ENTRY_SIZE)
#define DEFAULT_STACK_SIZE 2 * 1024 * 1024
#define STACK_START 0xc0000000
#define STACK_END (STACK_START - DEFAULT_STACK_SIZE)
#define MEMORY_REGION_MAPPED 0
#define MEMORY_REGION_UNMAPPED 1
#define MAX_MEMORY_ADDR 0xffffffff
#define MAX_AVAILABLE_USER_MEM (MAX_MEMORY_ADDR - USER_MEM_START)

#define PAGE_DIR_ENTRY_DEFAULT 0x00000002
#define PAGE_TABLE_ENTRY_DEFAULT 0x00000002

#define GET_ADDR_FROM_ENTRY(addr) (((unsigned int)addr)&0xFFFFF000)
#define GET_FLAGS_FROM_ENTRY(addr) (((unsigned int)addr)&0x00000FFF)

#define GET_PD_INDEX(addr) ((unsigned int)((int)(addr) & PAGE_DIRECTORY_MASK) >> 22)
#define GET_PT_INDEX(addr) ((unsigned int)((int)(addr) & PAGE_TABLE_MASK) >> 12)
#define KERNEL_MAP_NUM_ENTRIES (sizeof(direct_map) / sizeof(direct_map[0]))

void vm_init();

void *create_page_directory();

void free_page_directory(void *pd_addr);

void *clone_paging_info(int *pd);

void free_paging_info(int *pd);
 
int setup_page_table(simple_elf_t *se_hdr, void *pd_addr);

void set_kernel_pd();

void *get_kernel_pd();

void *get_dead_thr_kernel_stack();

void set_cur_pd(void *pd_addr);

int is_addr_cow(void *addr);

int handle_cow(void *addr);

void enable_paging();

int is_memory_range_mapped(void *base, int len);

int map_new_pages(void *base, int length);

int unmap_new_pages(void *base);

int is_memory_writable(void *ptr, int bytes);

int is_memory_writable(void *ptr, int bytes);

#endif /* __VM_H */
