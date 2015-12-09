/** Functions to implement the serial server 
 *
 * @file serial_server.c
 * 
 * @author Rohit Upadhyaya (rjupadhy) 
 * @author Prajwal Yadapadithaya (pyadapad)
 **/

#include <stdio.h>
#include <string.h>
#include <mutex.h>
#include <key_circular_buffer.h>
#include <circular_buffer.h>
#include <stdbool.h>
#include <syscall.h>
#include <ipc_client.h>
#include <ipc_server.h>
#include <errors.h>
#include <udriv_servers.h>
#include <ns16550.h>
#include <simics.h>
#include <thread.h>

#define BUF_LEN 1024

#define THREAD_STACK_SIZE 4096

#define COM1_PORT 0x3f8
#define COM2_PORT 0x2f8

#define COM1_ID_PORT (COM1_PORT + REG_INT_ID)
#define COM2_ID_PORT (COM2_PORT + REG_INT_ID)

#define DEV_RCV_INT 4
#define DEV_EMPTY_INT 2

#define NUM_ARGS 2
#define CARRIAGE_RETURN 13

char *print_server = "print_server";
char *readline_server = "readline_server";

const char *COM1 = "COM1";
const char *COM2 = "COM2";

const char new_line = '\n';
const char back_space = '\b';
const char space = ' ';

int com_device_id;

static message_struct_t print_buf;
static mutex_t print_buf_mutex;
static int buf_empty = 1;

extern void console_set_server(driv_id_t serv);
static int launch_server(char *server_name, char *args[]);
static void print_to_serial(int port);
static void *print_listener();
static void *send_readline();

/** @brief This function acts as the print server
 *
 *  This function is run as a separate thread by the serial
 *  server
 *  
 *  @return Does not return
 */
void *print_listener() {
	ipc_state_t* print_st;
	switch(com_device_id) {
		case 1: 
				if (ipc_server_init(&print_st, UDR_COM1_PRINT_SERVER) < 0) {
					panic("Couldn't start required server");
        		}
				break;
		case 2:
				if (ipc_server_init(&print_st, UDR_COM2_PRINT_SERVER) < 0) {
					panic("Couldn't start required server");
        		}
				break;
		default:
				break;
	}
	while(true) {
		driv_id_t sender;
		char buf[BUF_LEN];
		int len;
		if ((len = ipc_server_recv(print_st, &sender, &buf,
                                    BUF_LEN, true)) < 0) {
			ipc_server_cancel(print_st);
			panic("Error");
		}
		int i;
		mutex_lock(&print_buf_mutex);
		for(i = 0; i < len; i++) {
			if(buf[i] == new_line) {
				add_message(&print_buf, CARRIAGE_RETURN);
				add_message(&print_buf, new_line);
			}
    		else if (buf[i] == back_space) {
				add_message(&print_buf, back_space);
				add_message(&print_buf, space);
				add_message(&print_buf, back_space);
    		}
			else {
				add_message(&print_buf, buf[i]);
			}
		}
		if(buf_empty) {
			switch(com_device_id) {
				case 1: print_to_serial(COM1_PORT); 
						break;
				case 2: print_to_serial(COM2_PORT); 
						break;
				default: break;
			}
		}
		mutex_unlock(&print_buf_mutex);
	}
	return NULL;
}

int main(int argc, char *argv[]) {

	if(argc != NUM_ARGS) {
		return ERR_FAILURE;
	}
	if(strcmp(argv[1], COM1) != 0 && strcmp(argv[1], COM2) != 0) {
		return ERR_FAILURE;
	}
	if(strcmp(argv[1], COM1) == 0) {
		com_device_id = 1;
	}
	else if(strcmp(argv[1], COM2) == 0) {
		com_device_id = 2;
	}

	/* Initialize the readline and print servers for the serial device */
	char *readline_args[] = {readline_server, argv[1], 0};
	if(launch_server(readline_server, readline_args) < 0) {
		return ERR_FAILURE;
	}
	
	/* Initialize the print buffer */
	init_msg_data(&print_buf);
	mutex_init(&print_buf_mutex);

	/* Create a thread to listen to print requests */
	thr_init(THREAD_STACK_SIZE);
	thr_create(print_listener, NULL);

	/* Register as a driver for the serial device */
	switch(com_device_id) {
		case 1:
			if(udriv_register(UDR_DEV_COM1, COM1_ID_PORT, 1) < 0) {
				return ERR_FAILURE;
			}
			console_set_server(UDR_COM1_PRINT_SERVER);
			break;
		case 2:
			if(udriv_register(UDR_DEV_COM2, COM2_ID_PORT, 1) < 0) {
				return ERR_FAILURE;
			}
			console_set_server(UDR_COM2_PRINT_SERVER);
			break;
		default:
			return ERR_FAILURE;
	}

	driv_id_t driv_recv;
	message_t msg_recv;
	unsigned int msg_size;
    while (true) {
		if (udriv_wait(&driv_recv, &msg_recv, &msg_size) < 0) {
			break;
		}
		if(msg_recv & DEV_EMPTY_INT) {
			mutex_lock(&print_buf_mutex);
			if(has_message(&print_buf)) {
				switch(com_device_id) {
					case 1: print_to_serial(COM1_PORT); 
							break;
					case 2: print_to_serial(COM2_PORT); 
							break;
					default: break;
				}			
			}
			else {
				buf_empty = 1;
			}
			mutex_unlock(&print_buf_mutex);
			continue;
		}
		unsigned char c = ' ';
		switch(com_device_id) {
			case 1: if(udriv_inb(COM1_PORT, &c) < 0) {
						lprintf("inb failed");
					}
					break;
			case 2: if(udriv_inb(COM2_PORT, &c) < 0) {
						lprintf("inb failed");
					}
					break;
			default: break;
		}
		mutex_lock(&print_buf_mutex);
		if(c == CARRIAGE_RETURN) {
			add_keystroke(new_line);
			add_message(&print_buf, CARRIAGE_RETURN);
			add_message(&print_buf, new_line);
		}
		else {
			if(c == back_space && !has_key()) {
				mutex_unlock(&print_buf_mutex);
				continue;
			}
			add_keystroke(c);
			if(c == back_space) {
				add_message(&print_buf, back_space);
				add_message(&print_buf, space);
				add_message(&print_buf, back_space);
			}
			else {
				add_message(&print_buf, c);
			}
		}
        switch(com_device_id) {
            case 1: print_to_serial(COM1_PORT); 
					break;
            case 2: print_to_serial(COM2_PORT); 
					break;
            default: break;
        }
		mutex_unlock(&print_buf_mutex);
		if(c == CARRIAGE_RETURN) {
			int tid = thr_create(send_readline, NULL);
			thr_join(tid, NULL);
		}
    }

	return ERR_FAILURE;
}

/** @brief Function to launch new servers (print and readline)
 *
 *  @param server_name Name of the server to be launched
 *	@param args Arguments to the server
 *
 *	@return 0 on success. -ve number of failure
 */
int launch_server(char *server_name, char *args[]) {
	int server_tid = fork();
	if(server_tid < 0) {
		return ERR_FAILURE;
	}
	if(!server_tid) {
		if(exec(server_name, args) < 0) {
			return ERR_FAILURE;
		}
	}
	return 0;
}

/** @brief Print a character to a serial console
 *
 *  Checks the print buffer and prints one character
 *  to the serial device
 *  
 *  @return void
 */
void print_to_serial(int port) {
	if(!has_message(&print_buf)) {
		return;
	}
	char c = get_nextmsg(&print_buf);
	buf_empty = 0;	
    udriv_outb(port, c);
}

/** @brief Function to send the next line to the readline server
 *
 *  This is run in a new thread
 *
 *  @return NULL
 */
void *send_readline() {
	char buf[BUF_LEN];
	int num_char = get_nextline(buf, BUF_LEN);
	switch(com_device_id) {
		case 1:
			ipc_client_send_msg(COM1_READLINE_BUF_SERVER, 
								buf, num_char, NULL, 0);
			break;
		case 2:
			ipc_client_send_msg(COM2_READLINE_BUF_SERVER,
								buf, num_char, NULL, 0);
			break;
		default:
			break;
	}
	thr_exit((void *)0);
	return NULL;
}
