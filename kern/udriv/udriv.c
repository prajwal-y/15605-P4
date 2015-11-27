/** @file udriv.c
 *  @brief thread manipulation routines
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#include <common/malloc_wrappers.h>
#include <vm/vm.h>
#include <core/thread.h>
#include <udriv/circular_buffer.h>
#include <udriv/udriv.h>
#include <sync/mutex.h>
#include <cr.h>
#include <seg.h>
#include <syscall.h>
#include <stddef.h>
#include <simics.h>
#include <udriv_kern.h>
#include <udriv_registry.h>
#include <list/list.h>
#include <common/assert.h>
#include <common/errors.h>
#include <core/scheduler.h>

#define HASHMAP_SIZE PAGE_SIZE

static int next_unused_udriv_id;
static mutex_t next_mutex;
static mutex_t map_mutex;
static list_head udriv_map[HASHMAP_SIZE];

static void init_udriv_map();
static void add_udriv_to_map(udriv_struct_t *thr);
static udriv_struct_t *create_udriv();
static udriv_struct_t *get_udriv_from_id(int driver_id);
static void remove_udriv_from_map(int driver_id);
static int validate_port(driv_id_t driver_id, int port);

/** @brief Initializes the user driver creation module
 *
 *  @return Void
 */
void udriv_init() {
    next_unused_udriv_id = UDR_MIN_ASSIGNMENT + 1;
	mutex_init(&next_mutex);
    init_udriv_map();
}

/** @brief Function to register the current thread to be the 
 *  driver for the hardware device/software server passed as
 *  the argument
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int Registered driver ID on success. -ve number on
 *  failure
 */
int handle_udriv_register(void *arg_packet) {
	driv_id_t driver_id = (driv_id_t)(*((int *)arg_packet));
	unsigned int in_port = (unsigned int)(*((int *)arg_packet + 1));
	unsigned int in_bytes = (unsigned int)(*((int *)arg_packet + 2));
	
	/* Argument validation */
	if(driver_id < 0 || driver_id >= UDR_MIN_ASSIGNMENT) {
		return ERR_INVAL;
	}
	udriv_struct_t *udriv = get_udriv_from_id(driver_id);
	if(udriv != NULL) {
		return ERR_INVAL;
	}

	if(driver_id < UDR_MAX_HW_DEV) {
		if(validate_port(driver_id, in_port) < 0) {
			return ERR_INVAL;
		}
		if(in_bytes != 0 && in_bytes != 1) {
			return ERR_INVAL;
		}
	} else {
		if(in_bytes < 0 || in_bytes > sizeof(message_t)) {
			return ERR_INVAL;
		}
	}

	if(driver_id == UDR_ASSIGN_REQUEST) {
		mutex_lock(&next_mutex);
		driver_id = next_unused_udriv_id++;
		mutex_unlock(&next_mutex);
	}

	udriv_struct_t *driv = create_udriv(driver_id);
	if(driv == NULL) {
		return ERR_FAILURE;
	}
	return driv->id;
}

/** @brief Function to deregister a driver from a particular thread
 *
 *  @param driver_id The driver to be deregistered.
 *
 *  @return void
 */
void handle_udriv_deregister(driv_id_t driver_id) {
	thread_struct_t *curr_thread = get_curr_thread();
	udriv_struct_t *udriv = get_udriv_from_id(driver_id);
	if(udriv == NULL || udriv->reg_tid != curr_thread->id) {
		return;
	}

	del_entry(&udriv->thr_link);
	remove_udriv_from_map(driver_id);

	sfree(udriv, sizeof(udriv_struct_t));	
}

/** @brief Function to send an interrupt to the registered
 *  driver
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int 0 on success. -ve number on failure
 */
int handle_udriv_send(void *arg_packet) {
	return ERR_FAILURE;
}

/** @brief Function to collect an interrupt from a device
 * registered to the calling thread
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int 0 on success. -ve number on failure
 */
int handle_udriv_wait(void *arg_packet) {
	return ERR_FAILURE;
}

/** @brief Function to read the result of calling inb()
 *  on the I/O port.
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int 0 on success. -ve number on failure
 */
int handle_udriv_inb(void *arg_packet) {
	return ERR_FAILURE;
}

/** @brief Function to write the contents to the specified
 *  I/O port.
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int 0 on success. -ve number on failure
 */
int handle_udriv_outb(void *arg_packet) {
	return ERR_FAILURE;
}

/** @brief Function to map len bytes of physical memory to
 *  a specified virtual address for purposes of performing
 *  memory mapped I/O.
 *
 *  @param arg_packet Argument from the system call
 *
 *  @return int 0 on success. -ve number on failure
 */
int handle_udriv_mmap(void *arg_packet) {
	return ERR_FAILURE;
}

/* --------------- Static local functions ----------------*/

/** @brief Function to validate a port for a given driver ID.
 *
 *  @param driv_id_t Driver for which the port must be validated
 *  @param port Port to be validated
 *
 *  @return 0 if port is valid. -ve number if invalid
 */
int validate_port(driv_id_t driver_id, int port) {
	int i;
	for(i = 0; i < device_table_entries; i++) {
		dev_spec_t device = device_table[i];
		if(driver_id == device.id) {
			int j;
			for(j = 0; j < device.port_regions_cnt; j++) {
				if(port >= device.port_regions[j].base &&
					(port <= device.port_regions[j].base + 
							 device.port_regions[j].len)) {
					break;
				}
			}
			if(j == device.port_regions_cnt) {
				return ERR_INVAL;
			}
		}
	}
	return 0;
}

/** @brief Initialize the buckets of the hash map
 *
 *  Hash map mapping driver id to udriv_struct. The super complicated hash function
 *  used is udriv_id % HASHMAP_SIZE. Each bucket is a linked list to handle
 *  collisions.
 *  
 *  @return void
 */
void init_udriv_map() {
    int i;
    for (i = 0; i < HASHMAP_SIZE; i++) {
        init_head(&udriv_map[i]);
    }
    mutex_init(&map_mutex);
}

/** @brief add a udriv struct to the hashmap
 *
 *  calculate the index of the udriv using udriv->id % HASHMAP_SIZE
 *  Add it to tail of the bucket
 *
 *  @param udriv udriv_struct_t to be added to hashmap
 *  @return void
 */
void add_udriv_to_map(udriv_struct_t *udriv) {
    int index = udriv->id % HASHMAP_SIZE;
    mutex_lock(&map_mutex);
    add_to_tail(&udriv->map_link, &udriv_map[index]);
    mutex_unlock(&map_mutex);
}

/** @brief return udriv struct for a given driver id
 *
 *  @param driver_id driver id of driver struct we wish to retrieve
 *  @return udriv_struct_t* udriv struct corresponding to the driver id
 *                            null if not found
 */
udriv_struct_t *get_udriv_from_id(int driver_id) {
    int index = driver_id % HASHMAP_SIZE;
    list_head *bucket_head = &udriv_map[index];
    mutex_lock(&map_mutex);
    list_head *udriv_node = get_first(bucket_head);
	while(udriv_node != NULL && udriv_node != bucket_head) {
        udriv_struct_t *udriv = get_entry(udriv_node, udriv_struct_t, 
                                          map_link);
        if (udriv->id == driver_id) {
            mutex_unlock(&map_mutex);
            return udriv;
        }
		udriv_node = udriv_node->next;
	}
    mutex_unlock(&map_mutex);
    return NULL;
}

/** @brief remove udriv struct from hashmap for given driver id
 *
 *  @param driver_id driver id of driver to be removed from driver map
 *  @return void
 */
void remove_udriv_from_map(int driver_id) {
    int index = driver_id % HASHMAP_SIZE;
    list_head *bucket_head = &udriv_map[index];
    mutex_lock(&map_mutex);
    list_head *udriv_node = get_first(bucket_head);
	while(udriv_node != NULL && udriv_node != bucket_head) {
        udriv_struct_t *udriv = get_entry(udriv_node, udriv_struct_t, 
                                          map_link);
        if (udriv->id == driver_id) {
            del_entry(udriv_node);
            mutex_unlock(&map_mutex);
            return;
        }
		udriv_node = udriv_node->next;
	}
    mutex_unlock(&map_mutex);
}

/** @brief create a new user driver
 *
 *  Create udriv and add mapping in the hash map
 *
 *  @param driver_id ID of the driver to be created
 *  @return udriv_struct_t the newly created udriv struct. NULL on failure
 */
udriv_struct_t *create_udriv(driv_id_t driver_id) {

    /* Create the udriv struct */
	udriv_struct_t *udriv = (udriv_struct_t *)smalloc(sizeof(udriv_struct_t));
    if(udriv == NULL) {
        return NULL;
    }

	thread_struct_t *curr_thread = get_curr_thread();
	kernel_assert(curr_thread != NULL);

    udriv->id = driver_id;
	udriv->reg_tid = curr_thread->id;

	init_msg_data(&udriv->msg_data);

    /* Add udriv to hash map */
    add_udriv_to_map(udriv);

	/* Add to the list of drivers in the thread */
	add_to_tail(&udriv->thr_link, &curr_thread->udriv_list_link);

    return udriv;
}
