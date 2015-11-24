/** @file vm.c
 *  @brief implementation of virtual memory functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <vm/vm.h>
#include <common/malloc_wrappers.h>
#include <util.h>
#include <string.h>
#include <elf_410.h>
#include <common_kern.h>
#include <cr.h>
#include <stddef.h>
#include <page.h>
#include <simics.h>
#include <seg.h>
#include <asm/asm.h>
#include <sync/mutex.h>
#include <common/errors.h>
#include <common/assert.h>
#include <allocator/frame_allocator.h>

#define USER_PD_ENTRY_FLAGS PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | USER_MODE
#define SET_NEWPAGE_START(x) (((unsigned int)(x) & 0xfffff3ff) | NEWPAGE_START)
#define SET_NEWPAGE_END(x) (((unsigned int)(x) & 0xfffff3ff) | NEWPAGE_END)
#define GET_NEWPAGE_FLAGS(x) (((unsigned int)(x) & 0x00000c00))
#define IS_NEWPAGE_START(x) ((unsigned int)(x) & NEWPAGE_START)
#define IS_NEWPAGE_PAGE(x) ((unsigned int)(x) & NEWPAGE_PAGE)
#define IS_NEWPAGE_END(x) ((unsigned int)(x) & NEWPAGE_END)

static int *frame_ref_count;
static void *kernel_pd;
static void *dead_thr_kernel_stack;

static void init_frame_ref_count();
static void zero_fill(void *addr, int size);
static void direct_map_kernel_pages(void *pd_addr);
static void setup_direct_map();
static void setup_kernel_pd();
static int map_text_segment(simple_elf_t *se_hdr, void *pd_addr);
static int map_data_segment(simple_elf_t *se_hdr, void *pd_addr);
static int map_rodata_segment(simple_elf_t *se_hdr, void *pd_addr);
static int map_bss_segment(simple_elf_t *se_hdr, void *pd_addr);
static int map_stack_segment(void *pd_addr);
static int map_segment(void *start_addr, unsigned int length, int *pd_addr, int flags);
static void *direct_map[USER_MEM_START / (PAGE_SIZE * NUM_PAGE_TABLE_ENTRIES)];

static void *create_page_table();
static void free_page_table(int *pt);
static void make_pages_cow(int *pd);
static void make_pt_cow(int *pt);
static void increment_ref_count(int *pd);
static void enable_page_pinning();

/** @brief initialize the virtual memory system
 *
 *  Set up the direct map for kernel memory and any other
 *  initialization routines
 *
 *  @return void
 */
void vm_init() {
    setup_direct_map();
    setup_kernel_pd();
    set_kernel_pd();
    enable_paging();
	init_frame_ref_count();
    enable_page_pinning();
}

/** @brief Function to set the the special kernel page
 *         directory
 *
 *  @return void
 */
void setup_kernel_pd() {
    kernel_pd = create_page_directory();
    kernel_assert(kernel_pd != NULL);
    dead_thr_kernel_stack = (void *)smalloc(PAGE_SIZE);
	kernel_assert(dead_thr_kernel_stack != NULL);
}

/** @brief Initializes the array which stored the 
 * 		   reference count for physical frames
 *  @return void
 */
void init_frame_ref_count() {
	int size = FREE_FRAMES_COUNT*sizeof(int);
    frame_ref_count = (int *)smalloc(size);
	kernel_assert(frame_ref_count != NULL);
	memset(frame_ref_count, 0, size);
}

/** @brief Set the current page directory to kernel page
 *   directory
 * 
 *  @return void
 */
void set_kernel_pd() {
    set_cur_pd(kernel_pd);
}

/** @brief Gets the address of the kernel page directory
 *
 *  @return void* Address of the kernel PD
 */
void *get_kernel_pd() {
	return kernel_pd;
}

/** @brief Gets the address of the dead thread kernel stack
 *
 *  @return void* Address of the special kernel stack for dead
 *  threads
 */
void *get_dead_thr_kernel_stack() {
	return ((char *)dead_thr_kernel_stack + PAGE_SIZE - 1);
}

/** @brief Sets the control register %cr3 with the given
 * address of the page directory address.
 *
 * @param pd_addr The page directory address that needs to
 * be set in %cr3
 *
 * @return Void
 */
void set_cur_pd(void *pd_addr) {
	set_cr3((uint32_t)pd_addr);
}

/** @brief enable VM 
 *
 *  Set bit 31 of cr0
 *
 *  @return void
 */
void enable_paging() {
    unsigned int cr0 = get_cr0();
    cr0 = cr0 | CR0_PG;
    set_cr0(cr0);
}

/** @brief Function to enable setting of 
 *  global flag.
 * 
 *  @return void
 */
void enable_page_pinning() {
    unsigned int cr4 = get_cr4();
    cr4 = cr4 | CR4_PGE;
    set_cr4(cr4);
}

/** @brief create a new page directory 
 *
 *  This function will allocate a new kernel physical frame for 
 *  a page directory and return the address of this physical frame.
 *  The frame will be initialized with default flags. If there are 
 *  no more physical frames available this function will return null.
 *  
 *  @return address of the frame containing the page directory.
 *          NULL on failure
 */
void *create_page_directory() {
	int *frame_addr = (int *) smemalign(PAGE_SIZE, PAGE_SIZE);
    if(frame_addr == NULL) {
        return NULL;
    }
    direct_map_kernel_pages(frame_addr);
	int i;
	for(i=KERNEL_MAP_NUM_ENTRIES; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		frame_addr[i] = PAGE_DIR_ENTRY_DEFAULT;
	}
    return (void *)frame_addr;
}
 
/** @brief free a page directory 
 *
 *  frees the specified page directory using sfree
 *  
 *  @return void
 */
void free_page_directory(void *pd_addr) {
    if (pd_addr == NULL) {
        return;
    }
	sfree(pd_addr, PAGE_SIZE);
}

/** @brief create a new page table
 *
 *  This function will allocate a new kernel physical frame for 
 *  a page table and return the address of this physical frame.
 *  The frame will be initialized with default flags. If there are 
 *  no more physical frames available this function will return null.
 *  
 *  @return address of the frame containing the page table. NULL on failure
 */
void *create_page_table() {
	int *frame_addr = (int *) smemalign(PAGE_SIZE, PAGE_SIZE);
    if(frame_addr == NULL) {
        return NULL;
    }
	int i;
	for(i=0; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		frame_addr[i] = PAGE_TABLE_ENTRY_DEFAULT;
	}
    return (void *)frame_addr;
}

/** @brief Creates a clone of the page table
 * 
 * 	@param pt Address of the page table to be cloned
 *  
 *  @return Address of the new page table
 */
void *clone_page_table(void *pt) {
	if(pt == NULL) {
		return NULL;
	}
	void *new_pt = create_page_table();
	if(new_pt == NULL) {
		return NULL;
	}
	memcpy(new_pt, pt, PAGE_SIZE);
	increment_ref_count(new_pt);
	return new_pt;
}

/** @brief free a page table
 *
 *  frees the specified page table using sfree
 *  
 *  @return void
 */
void free_page_table(int *pt) {
    if (pt == NULL) {
        return;
    }
	int i;
	for(i=0; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		if(pt[i] == PAGE_TABLE_ENTRY_DEFAULT) {
			continue;
		}
		void *frame_addr = (void *)GET_ADDR_FROM_ENTRY(pt[i]);
		lock_frame(frame_addr);
		frame_ref_count[FRAME_INDEX(frame_addr)]--;
		kernel_assert(frame_ref_count[FRAME_INDEX(frame_addr)] >= 0);
		if(frame_ref_count[FRAME_INDEX(frame_addr)] == 0) {
			deallocate_frame(frame_addr);
		}
		unlock_frame(frame_addr);
	}
	sfree(pt, PAGE_SIZE);
}

/** @brief Creates a copy of the given page directory and
 * 	the corresponding valid page tables.
 *
 *  @param pd Address of the page directory
 *
 *  @return Address of the clone of the given page directory
 */
void *clone_paging_info(int *pd) {
	if(pd == NULL) {
		return NULL;
	}
	int *new_pd = create_page_directory();
	if(new_pd == NULL) {
		return NULL;
	}
	int i;
	for(i=KERNEL_MAP_NUM_ENTRIES; i<NUM_PAGE_TABLE_ENTRIES; i++) {
        if(pd[i] != PAGE_DIR_ENTRY_DEFAULT) {
			void *new_pt = clone_page_table((void *)GET_ADDR_FROM_ENTRY(pd[i]));
			if(new_pt == NULL) {
				free_paging_info(new_pd);
				return NULL;
			}
			new_pd[i] = (unsigned int)new_pt | GET_FLAGS_FROM_ENTRY(pd[i]);
		}
    }
	make_pages_cow(pd);
	make_pages_cow(new_pd);
	return new_pd;
}
      
/** @brief Frees the paging frames of a given page directory
 *
 *  @param pd Address of the page directory
 *
 *  @return Void
 */
void free_paging_info(int *pd) {
	if(pd == NULL) {
		return;
	}
	int i;
	for(i=KERNEL_MAP_NUM_ENTRIES; i<NUM_PAGE_TABLE_ENTRIES; i++) {
        if(pd[i] != PAGE_DIR_ENTRY_DEFAULT) {
			free_page_table((void *)GET_ADDR_FROM_ENTRY(pd[i]));	
		}
    }
	free_page_directory(pd);
}

/** @brief Increments the reference count for all the physical frames
 *  allocated for a given page table
 *
 *  @return void
 */
void increment_ref_count(int *pt) {
	if(pt == NULL) {
		return;
	}
	int i;
	for(i=0; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		if(pt[i] != PAGE_TABLE_ENTRY_DEFAULT) {
			void *frame_addr = (void *)GET_ADDR_FROM_ENTRY(pt[i]);
			lock_frame(frame_addr);
			frame_ref_count[FRAME_INDEX(frame_addr)]++;
			unlock_frame(frame_addr);
		}
	}
}

/*********************COPY-ON-WRITE FUNCTIONS***************************/

/** @brief Function to make the pages for a task copy-on-write
 * 
 *  @param pd Page directory
 *
 *  @return Void
 */
void make_pages_cow(int *pd) {
	if(pd == NULL) {
		return;
	}
	int i;
	for(i=KERNEL_MAP_NUM_ENTRIES; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		if(pd[i] != PAGE_DIR_ENTRY_DEFAULT) {
			make_pt_cow((int *)GET_ADDR_FROM_ENTRY(pd[i]));	
		}
	}
}

/** @brief Functions to make a page table COW
 *
 *  This function iterates through each page table entry and makes
 *  the writable entries read only along with adding the COW flag.
 *
 *  @return void
 */
void make_pt_cow(int *pt) {
	if(pt == NULL) {
		return;
	}
	int i;
	for(i=0; i<NUM_PAGE_TABLE_ENTRIES; i++) {
		if(pt[i] == PAGE_TABLE_ENTRY_DEFAULT) {
			continue;
		}
		if(GET_FLAGS_FROM_ENTRY(pt[i]) & READ_WRITE_ENABLE) {
			pt[i] = (pt[i] | COW_MODE) & WRITE_DISABLE_MASK;
		}
	}
}

/** @brief Function to check if a particular address is COW
 *
 *  @param addr Virtual address to be checked.
 *
 *  @return 1 if COW, 0 if not
 */
int is_addr_cow(void *addr) {
	if((unsigned int)addr < USER_MEM_START) {
		return 0;
	}
	int *pd = (void *)get_cr3();
	int pd_index = GET_PD_INDEX(addr);
	int pt_index = GET_PT_INDEX(addr);
	int *pt = (int *)GET_ADDR_FROM_ENTRY(pd[pd_index]);
	if((pt[pt_index] | COW_MODE) && !(pt[pt_index] & READ_WRITE_ENABLE)
		&& (pt[pt_index]&PAGE_ENTRY_PRESENT)) {
		return 1;
	}
	return 0;
}

/** @brief Function to handle COW
 *
 *  This function allocates a new physical frame for the given virtual address
 *  and adjusts the frame reference count accordingly
 *
 *  @return int 0 on success. Negative number on failure
 */
int handle_cow(void *addr) {
	int *pd = (void *)get_cr3();
    int pd_index = GET_PD_INDEX(addr);
    int pt_index = GET_PT_INDEX(addr);
    int *pt = (int *)GET_ADDR_FROM_ENTRY(pd[pd_index]);
	void *frame_addr = (void *)GET_ADDR_FROM_ENTRY(pt[pt_index]);
    void *page_addr = (void *)((int)addr & PAGE_ROUND_DOWN);
	lock_frame(frame_addr);
	if(frame_ref_count[FRAME_INDEX(frame_addr)] == 1) {
		pt[pt_index] &= COW_MODE_DISABLE_MASK;
	} else {
        /* Copy the data from the old frame to a kernel frame */
		char frame_contents[PAGE_SIZE];
		memcpy(frame_contents, page_addr, PAGE_SIZE);

		void *new_frame = allocate_frame();
		if(new_frame == NULL) {
			unlock_frame(frame_addr);
			return ERR_FAILURE;
		}

		int flags = GET_FLAGS_FROM_ENTRY(pt[pt_index]);
		pt[pt_index] = (unsigned int)new_frame | flags;
		pt[pt_index] &= COW_MODE_DISABLE_MASK;
        set_cur_pd(pd);

        /* Copy data from kernel frame into the new allocated frame and free 
         * the kernel scratch space */
        memcpy(page_addr, frame_contents, PAGE_SIZE);

		/* Adjust reference counts */
		frame_ref_count[FRAME_INDEX(frame_addr)]--;

		lock_frame(new_frame);
		frame_ref_count[FRAME_INDEX(new_frame)]++;
		unlock_frame(new_frame);
	}
	unlock_frame(frame_addr);
	pt[pt_index] |= READ_WRITE_ENABLE;

	/* INVLPG is not working for some reason :( */
	set_cur_pd(pd);

	return 0;
}

/******************COPY-ON-WRITE FUNCTIONS END*************************/

/** @brief setup paging for a program
 *
 *  this function reads a simple_elf_t and creates mappings in the 
 *  page directory/page table for the regions found in the elf header.
 *  This function DOES NOT copy the data from the binary to these
 *  regions. That is the responsisbility of the loader. 
 *
 *  @param se_hdr pointer to a simple_elf_t containing info about the 
 *                program to be loaded
 *  @param pd_addr the address of the page directory obtained through
 *                 a previous call to create_page_directory
 *
 *  @return 0 on success. Negative number on failure
 */
int setup_page_table(simple_elf_t *se_hdr, void *pd_addr) {
    if(se_hdr == NULL || pd_addr == NULL) {
        return ERR_FAILURE;
    }
    int retval;
    if((retval = map_text_segment(se_hdr, pd_addr)) < 0) {
        return retval;
    }
    if((retval = map_data_segment(se_hdr, pd_addr)) < 0) {
        return retval;
    }
    if((retval = map_rodata_segment(se_hdr, pd_addr)) < 0) {
        return retval;
    }
    if((retval = map_bss_segment(se_hdr, pd_addr)) < 0) {
        return retval;
    }
    if((retval = map_stack_segment(pd_addr)) < 0) {
        return retval;
    }
    return 0;
}

/* ---------- Static local functions ----------- */

/** @brief zero fill a memory location
 *
 * @param addr the starting address of the memory location to be zeroed
 * @param size the number of bytes to be zeroed
 *
 * @return void
 */
void zero_fill(void *addr, int size) {
    if (addr == NULL) {
        return;
    }
    memset(addr, 0, size);
    return;
}

/** @brief direct map the kernel memory space
 *
 *  maps the lower 16 MB of the virtual memory to the lower 16 MB of
 *  physical memory. These pages are neither readable nor writable.
 *  All processes share the same page tables for the bottom 16 MB (i.e
 *  the page directory for all processes point to the same page tables for
 *  kernel memory). Therefore this function should be called once the mapping
 *  has been setup (setup_direct_map) and this simply copies the value in the
 *  direct_map array to the page directory.
 *
 * @param pd_addr the page directory where the direct mapping has to be 
 * 			performed
 *
 * @return void
 */
void direct_map_kernel_pages(void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE;
    int i;
    for (i = 0; i < KERNEL_MAP_NUM_ENTRIES; i++) {
        ((int *)pd_addr)[i] = (int)direct_map[i] | flags;
    }
}

/** @brief set up the direct map for kernel memory
 *
 *  get 4 frames required to map the bottom 16 MB of physical address space
 *  and write the page table entries corresponding to the 16 MB along with a
 *  no read/write protection policy. These 4 frames are stored in the array 
 *  direct_map which are used whenever a new page directory is initialized. 
 *  The page directory points to these 4 page tables
 *
 *  @return void
 */
void setup_direct_map() {
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | GLOBAL_PAGE_ENTRY;
    int i = 0, j = 0, mem_start = 0, page_table_entry;

    for (i = 0; i < KERNEL_MAP_NUM_ENTRIES; i++) {
        int *frame_addr = (int *)create_page_table();

        kernel_assert(frame_addr != NULL);

        for (j = 0; j < NUM_PAGE_TABLE_ENTRIES; j++) {
            page_table_entry = mem_start | flags;
            frame_addr[j] = page_table_entry;
            mem_start += PAGE_SIZE;
        }                
        direct_map[i] = frame_addr;
    }
}

/** @brief map the text segment into virtual memory
 *
 *  This function checks the address of the start of the text
 *  segment and length to define the number of physical frames
 *  required for the text segment. Then call a function which allocates
 *  free frames from the frame pool, sets up the mapping in the page directory.
 *
 *  @param se_hdr the parsed elf header
 *  @param pd_addr the address of the page directory frame
 *  @return int error code, 0 on success negative integer on failure
 */
int map_text_segment(simple_elf_t *se_hdr, void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | USER_MODE;
    return map_segment((void *)se_hdr->e_txtstart, se_hdr->e_txtlen, 
						pd_addr, flags);
}

/** @brief map the data segment into virtual memory
 *
 *  This function checks the address of the start of the data
 *  segment and length to define the number of physical frames
 *  required for the data segment. Then call a function which allocates
 *  free frames from the frame pool, sets up the mapping in the page directory.
 *
 *  @param se_hdr the parsed elf header
 *  @param pd_addr the address of the page directory frame
 *  @return int error code, 0 on success negative integer on failure
 */
int map_data_segment(simple_elf_t *se_hdr, void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | USER_MODE;
    return map_segment((void *)se_hdr->e_datstart, se_hdr->e_datlen, 
						pd_addr, flags);
}

/** @brief map the rodata segment into virtual memory
 *
 *  This function checks the address of the start of the rodata
 *  segment and length to define the number of physical frames
 *  required for the rodata segment. Then call a function which allocates
 *  free frames from the frame pool, sets up the mapping in the page directory.
 *
 *  @param se_hdr the parsed elf header
 *  @param pd_addr the address of the page directory frame
 *  @return int error code, 0 on success negative integer on failure
 */
int map_rodata_segment(simple_elf_t *se_hdr, void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | USER_MODE;
    return map_segment((void *)se_hdr->e_rodatstart, 
						se_hdr->e_rodatlen, pd_addr, flags);
}

/** @brief map the bss segment into virtual memory
 *
 *  This function checks the address of the start of the bss
 *  segment and length to define the number of physical frames
 *  required for the bss segment. Then call a function which allocates
 *  free frames from the frame pool, sets up the mapping in the page directory.
 *
 *  @param se_hdr the parsed elf header
 *  @param pd_addr the address of the page directory frame
 *  @return int error code, 0 on success negative integer on failure
 */
int map_bss_segment(simple_elf_t *se_hdr, void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | USER_MODE;
    return map_segment((void *)se_hdr->e_bssstart, se_hdr->e_bsslen, 
						pd_addr, flags);
}

/** @brief map the stack segment into virtual memory
 *
 *  @param pd_addr the address of the page directory frame
 *  @return int error code, 0 on success negative integer on failure
 */
int map_stack_segment(void *pd_addr) {
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | USER_MODE;
    return map_segment((char *)STACK_START - DEFAULT_STACK_SIZE + 1, 
						DEFAULT_STACK_SIZE, pd_addr, flags); 
}

/** @brief map new_pages into virtual memory
 *
 *  @param base the base of the new_pages region
 *  @param len the length of the new pages region
 *  @return int error code, 0 on success negative integer on failure
 */
int map_new_pages(void *base, int length) {
    int retval, pd_index, pt_index;
    int *pd_addr = (int *)get_cr3();
    int *pt_addr;
    int *end_addr = (int *)((char *)base + length - 1);
    int *end_frame = (int *)((int)end_addr & PAGE_ROUND_DOWN);
    int flags = PAGE_ENTRY_PRESENT | READ_WRITE_ENABLE | USER_MODE
                | NEWPAGE_PAGE;
    retval = map_segment((char *)base, length, pd_addr, flags); 
    if (retval < 0) {
        return retval;
    }

    pd_index = GET_PD_INDEX(end_frame);
    pt_index = GET_PT_INDEX(end_frame);
    pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
    pt_addr[pt_index] = SET_NEWPAGE_END(pt_addr[pt_index]);

    pd_index = GET_PD_INDEX(base);
    pt_index = GET_PT_INDEX(base);
    pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
    pt_addr[pt_index] = SET_NEWPAGE_START(pt_addr[pt_index]);

	/* INVLPG is not working for some reason :( */
	set_cur_pd(pd_addr);

    return 0;
}

/** @brief unmap new_pages from virtual memory
 *
 *  @param base the base of the new_pages region
 *  @return int error code, 0 on success negative integer on failure
 */
int unmap_new_pages(void *base) {
    int pd_index, pt_index;
    int *pd_addr = (int *)get_cr3();
    int *pt_addr;
    void *frame_addr;

    pd_index = GET_PD_INDEX(base);
    pt_index = GET_PT_INDEX(base);
    pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
    if (GET_NEWPAGE_FLAGS(pt_addr[pt_index]) != NEWPAGE_START) {
        return ERR_INVAL;
    }

    frame_addr = (void *)GET_ADDR_FROM_ENTRY(pt_addr[pt_index]);
	lock_frame(frame_addr);
	frame_ref_count[FRAME_INDEX(frame_addr)]--;
	if(frame_ref_count[FRAME_INDEX(frame_addr)] == 0) {
    	deallocate_frame(frame_addr); 
	}
	unlock_frame(frame_addr);
	pt_addr[pt_index] = PAGE_TABLE_ENTRY_DEFAULT;
    
    base = (char *)base + PAGE_SIZE;
    pd_index = GET_PD_INDEX(base);
    pt_index = GET_PT_INDEX(base);
    pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
    while((GET_NEWPAGE_FLAGS(pt_addr[pt_index]) == NEWPAGE_PAGE) 
          || (GET_NEWPAGE_FLAGS(pt_addr[pt_index]) == NEWPAGE_END)) { 
        frame_addr = (void *)GET_ADDR_FROM_ENTRY(pt_addr[pt_index]);
		lock_frame(frame_addr);
		frame_ref_count[FRAME_INDEX(frame_addr)]--;
		if(frame_ref_count[FRAME_INDEX(frame_addr)] == 0) {
			deallocate_frame(frame_addr); 
		}
		unlock_frame(frame_addr);
        pt_addr[pt_index] = PAGE_TABLE_ENTRY_DEFAULT;
        base = (char *)base + PAGE_SIZE;
        pd_index = GET_PD_INDEX(base);
        pt_index = GET_PT_INDEX(base);
        pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
    }

	/* INVLPG is not working for some reason :( */
	set_cur_pd(pd_addr);

    return 0;
}

/** @brief map a segment into memory
 *
 *  This function takes the starting virtual address and the length. Then
 *  calculate the total number of frames required for this range of memory 
 *  allocate those many frames and set up the mapping in the page directory.
 *
 *  @param start_addr the start of the virtual address
 *  @param length the length of this memory segment
 *  @param pd_addr the address of the page directory
 *
 *  @return int error code, 0 on success negative integer on failure
 */
int map_segment(void *start_addr, unsigned int length, int *pd_addr, int flags) {
    set_cur_pd(pd_addr);
    void *end_addr = (char *)start_addr + length;
    int pd_index, pt_index;
    int *pt_addr;

    start_addr = (void *)((int)start_addr & PAGE_ROUND_DOWN);
    while (start_addr < end_addr) {
        pd_index = GET_PD_INDEX(start_addr);
        pt_index = GET_PT_INDEX(start_addr);
        if (pd_addr[pd_index] == PAGE_DIR_ENTRY_DEFAULT) { /* Page directory entry absent */
            void *new_pt = create_page_table();
            if (new_pt != NULL) {
                pd_addr[pd_index] = (unsigned int)new_pt | USER_PD_ENTRY_FLAGS;
            }
            else {
                return ERR_NOMEM;
            }
        }
        pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
        if (pt_addr[pt_index] == PAGE_TABLE_ENTRY_DEFAULT) { /* Page table entry absent */
            /* Need to allocate frame from user free frame pool */
            void *new_frame = allocate_frame();
            if (new_frame != NULL) {
				lock_frame(new_frame);
                frame_ref_count[FRAME_INDEX(new_frame)]++;
				unlock_frame(new_frame);
                pt_addr[pt_index] = (unsigned int)new_frame | flags;
                zero_fill(start_addr, PAGE_SIZE);
            }
            else {
                return ERR_NOMEM;
            }
        }
        start_addr = (char *)start_addr + PAGE_SIZE;
    }
    return 0;
}

/** @brief check if a memory region is mapped in the current 
 *         process's usable address space
 *
 *  This function makes use of the paging info of the current process
 *  to determine if any part of the memory range passed in has already
 *  been mapped.
 *
 *  @param base the base of the memory range to be checked
 *  @param len the length of the memory range to be checked
 *
 *  @return MEMORY_REGION_MAPPED if any portion of the range passed in is 
 *          already mapped or is in kernel address space. 
 *          MEMORY_REGION_UNMAPPED if the entire range is not mapped in the
 *          process address space and is not in the kernel address space. 
 *          ERR_INVAL if illegal arguments are passed in.
 */
int is_memory_range_mapped(void *base, int len) {
    if (base == NULL || base > (void *)MAX_MEMORY_ADDR ||
        len <= 0 || len > (MAX_AVAILABLE_USER_MEM)) {
        return ERR_INVAL;
    } 
    if (base < (void *)USER_MEM_START) {
        return MEMORY_REGION_MAPPED;
    }
    void *end_addr = (char *)base + len;
    int *pd_addr = (int *)get_cr3();
    int *pt_addr;
    int pd_index, pt_index;

    base = (void *)((int)base & PAGE_ROUND_DOWN);
    while (base <= end_addr) {
        pd_index = GET_PD_INDEX(base);
        pt_index = GET_PT_INDEX(base);
        if (pd_addr[pd_index] != PAGE_DIR_ENTRY_DEFAULT) {
            pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
            if (pt_addr[pt_index] != PAGE_TABLE_ENTRY_DEFAULT) {
                return MEMORY_REGION_MAPPED;
            }
        }
        base = (char *)base + PAGE_SIZE;
    }

    return MEMORY_REGION_UNMAPPED;
}

/** @brief check if memory location is user writable
 *
 *  Check if memory location pointed to by ptr can be written to by user
 *
 *  @param ptr memory address
 *  @param bytes number of bytes that have to be written
 *  @return 0 on success, -ve integer if cannot be written to
 */
int is_memory_writable(void *ptr, int bytes) {
    int *pd_addr = (int *)get_cr3();
    int *pt_addr;
    int pd_index, pt_index;
        
    pd_index = GET_PD_INDEX(ptr);
    pt_index = GET_PT_INDEX(ptr);
    if (pd_addr[pd_index] != PAGE_DIR_ENTRY_DEFAULT) {
        pt_addr = (int *)GET_ADDR_FROM_ENTRY(pd_addr[pd_index]);
        if (pt_addr[pt_index] != PAGE_TABLE_ENTRY_DEFAULT) {
            if (pt_addr[pt_index] & READ_WRITE_ENABLE) {
                return 0;
            }
        }
    }
    return ERR_INVAL;
}

