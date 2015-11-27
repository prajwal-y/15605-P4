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
#include <udriv_registry.h>
#include <udriv/circular_buffer.h>

typedef struct udriv_struct {
    driv_id_t id;				/* A device/software server driver ID */
    int reg_tid;				/* Registered thread ID */
	list_head map_link;			/* Link in the map */
	list_head thr_link;			/* Link in the thread */
	message_struct_t msg_data;	/* Messages of the interrupts of the driver */
} udriv_struct_t;

void udriv_init();

int handle_udriv_register(void *arg_packet);
void handle_udriv_deregister(driv_id_t driver_id);
int handle_udriv_send(void *arg_packet);
int handle_udriv_wait(void *arg_packet);
int handle_udriv_inb(void *arg_packet);
int handle_udriv_outb(void *arg_packet);
int handle_udriv_mmap(void *arg_packet);

#endif  /* __UDRIV_H */
