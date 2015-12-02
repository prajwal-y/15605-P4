/** Functions to implement the serial server 
 *
 * @file serial_server.c
 * 
 * @author Rohit Upadhyaya (rjupadhy) 
 * @author Prajwal Yadapadithaya (pyadapad)
 **/

#include <stdio.h>
#include <string.h>
#include <key_circular_buffer.h>
#include <stdbool.h>
#include <syscall.h>
#include <simics.h>
#include <ipc_client.h>
#include <errors.h>
#include <udriv_servers.h>

#define BUF_LEN 1024

#define COM1_PORT 0x3f8
#define COM2_PORT 0x2f8

#define NUM_ARGS 2

char *print_server = "print_server";
char *readline_server = "readline_server";

const char *COM1 = "COM1";
const char *COM2 = "COM2";

int com_device_id;

static int launch_server(char *server_name, char *args[]);

int main(int argc, char *argv[]) {

	if(argc != NUM_ARGS) {
		lprintf("Number of arguments incorrect");
		return ERR_FAILURE;
	}
	if(strcmp(argv[1], COM1) != 0 && strcmp(argv[1], COM2) != 0) {
		lprintf("Invalid argument %s", argv[1]);
		return ERR_FAILURE;
	}
	if(strcmp(argv[1], COM1) == 0) {
		com_device_id = 1;
	}
	else if(strcmp(argv[1], COM2) == 0) {
		com_device_id = 2;
	}

	lprintf("Trying to initialize serial server for %s", argv[1]);

	/* Initialize the readline and print servers for the serial device */
	char *readline_args[] = {readline_server, argv[1], 0};
	if(launch_server(readline_server, readline_args) < 0) {
		return ERR_FAILURE;
	}
	char *print_args[] = {print_server, argv[1], 0};
	if(launch_server(print_server, print_args) < 0) {
		return ERR_FAILURE;
	}

    int pid;
    if ((pid = fork()) != 0) {
        if (pid < 0) {
            lprintf("serial could not be started\n");
            return ERR_FAILURE;
        } else {
            lprintf("serial server started on pid %d\n", pid);
            return 0;
        }
    }    

	/* Register as a driver for the serial device */
	switch(com_device_id) {
		case 1:
			if(udriv_register(UDR_DEV_COM1, COM1_PORT, 1) < 0) {
				return ERR_FAILURE;
			}
			break;
		case 2:
			if(udriv_register(UDR_DEV_COM2, COM2_PORT, 1) < 0) {
				return ERR_FAILURE;
			}
			break;
		default:
			return ERR_FAILURE;
	}

	driv_id_t driv_recv;
	message_t msg_recv;
	unsigned int msg_size;
    while (true) {
		if (udriv_wait(&driv_recv, &msg_recv, &msg_size) < 0) {
			lprintf("Something bad happened");
			break;
		}
		lprintf("received interrupt from %d", (int)driv_recv);
		char c = (char)msg_recv;
		add_keystroke(c);
		if(c == '\n') {
			char buf[BUF_LEN + 1];
			get_nextline(buf, BUF_LEN);
			buf[BUF_LEN] = '\0'; //Should be null terminated
			switch(com_device_id) {
				case 1:
					ipc_client_send_str(COM1_READLINE_BUF_SERVER, 
										buf, NULL, 0);
					break;
				case 2:
					ipc_client_send_str(COM2_READLINE_BUF_SERVER,
										buf, NULL, 0);
                    break;
				default:
					break;
			}
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
		lprintf("Can't create %s", server_name);
		return ERR_FAILURE;
	}
	if(!server_tid) {
		if(exec(server_name, args) < 0) {
			return ERR_FAILURE;
		}
	}
	return 0;
}