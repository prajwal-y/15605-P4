/* @file print_server.c 
 * 
 * @author Rohit Upadhyaya (rjupadhy)
 * @author Prajwal Yadapadithaya (pyadapad)
 **/

#include <stdio.h>
#include <simics.h>
#include <string.h>
#include <stdbool.h>
#include <syscall.h>
#include <rand.h>
#include <ipc_server.h>
#include <errors.h>
#include <udriv_servers.h>

#define BUF_LEN 1024
#define NUM_ARGS 2

#define COM1_PORT 0x3f8
#define COM2_PORT 0x2f8

void print_to_serial_device(int port, char *buf);

int main(int argc, char *argv[]) {

	if(argc != NUM_ARGS) {
		lprintf("Number of arguments to readline is incorrect");
		return ERR_FAILURE;
	}

    ipc_state_t* server_st;
	if(strcmp(argv[1], "COM1") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM1_READLINE_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}
	else if(strcmp(argv[1], "COM2") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM2_READLINE_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}

    while (true) {
        driv_id_t sender;
		char buf[BUF_LEN];
        if (ipc_server_recv(server_st, &sender, &buf, BUF_LEN, true) < 0) {
			lprintf("Failed in server_st recv");
            ipc_server_cancel(server_st);
            return ERR_FAILURE;
        }

		if(strcmp(argv[1], "COM1") == 0) {
			print_to_serial_device(COM1_PORT, buf);	
		} else if(strcmp(argv[1], "COM2") == 0) {
			print_to_serial_device(COM2_PORT, buf);
		}
    }
}

/** @brief Function to print to a serial device
 *
 *  @param port Port to write to
 *  @param buf Characters to be printed (Should be null terminated
 *
 *  @return void
 */
void print_to_serial_device(int port, char *buf) {
	int i = 0;
	while(buf[i] != '\0') {
		udriv_outb(port, buf[i]);
		i++;
	}
}
