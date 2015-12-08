/** @file thread.h
 *  @brief prototypes of user driver routines
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __UDRIV_H
#define __UDRIV_H

#include <ureg.h>
#include <stdint.h>
#include <list/list.h>
#include <syscall.h>
#include <udriv_kern.h>
#include <sync/mutex.h>
#include <core/thread.h>
#include <udriv_registry.h>
#include <udriv/circular_buffer.h>

typedef struct udriv_struct {
    driv_id_t id;				/* A device/software server driver ID */
    int reg_tid;				/* Registered thread ID */
	list_head map_link;			/* Link in the map */
	list_head thr_link;			/* Link in the thread */
	message_struct_t msg_data;	/* Messages of the interrupts of the driver */
	unsigned int msg_size;		/* Size of the messages for the driver */
	mutex_t msg_mutex;			/* Mutex to protect accessing the message data */
    unsigned int in_bytes;      /* Number of bytes to be read from in_port */
    unsigned int in_port;       /* Port to be read from */
} udriv_struct_t;

/* Server table which stores well known servers for I/O permissions */
extern const dev_spec_t server_table[];
extern const int server_table_entries;

void udriv_init();

thread_struct_t *get_udriv_thread();

int handle_udriv_register(void *arg_packet);
void handle_udriv_deregister(driv_id_t driver_id);
int handle_udriv_send(void *arg_packet);
int handle_udriv_wait(void *arg_packet);
int handle_udriv_inb(void *arg_packet);
int handle_udriv_outb(void *arg_packet);
int handle_udriv_mmap(void *arg_packet);

int udriv_send_interrupt(driv_id_t driv_send, message_t msg_send, 
							unsigned int msg_size);
udriv_struct_t *get_udriv_from_id(int driver_id);

#endif  /* __UDRIV_H */
