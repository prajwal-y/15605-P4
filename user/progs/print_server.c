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

void print_to_serial_device(int port, char *buf, int len);

int main(int argc, char *argv[]) {

	lprintf("in print server");

	if(argc != NUM_ARGS) {
		lprintf("Number of arguments to print is incorrect");
		return ERR_FAILURE;
	}

    ipc_state_t* server_st;
	if(strcmp(argv[1], "COM1") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM1_PRINT_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}
	else if(strcmp(argv[1], "COM2") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM2_PRINT_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}

    while (true) {
        driv_id_t sender;
		char buf[BUF_LEN];
		int len;
        if ((len = ipc_server_recv(server_st, &sender, &buf, 
									BUF_LEN, true)) < 0) {
            ipc_server_cancel(server_st);
            return ERR_FAILURE;
        }
	
		if(strcmp(argv[1], "COM1") == 0) {
			print_to_serial_device(COM1_PORT, buf, len);
		} else if(strcmp(argv[1], "COM2") == 0) {
			print_to_serial_device(COM2_PORT, buf, len);
		}
    }
}

/** @brief Function to print to a serial device
 *
 *  @param port Port to write to
 *  @param buf Characters to be printed
 *  @param len Length of buf
 *
 *  @return void
 */
void print_to_serial_device(int port, char *buf, int len) {
	int i = 0;
	while(buf[i] != '\0' && i < len) {
		if(buf[i] == '\n') {
			udriv_outb(port, 13);
			udriv_outb(port, buf[i]);
		} 
        else if (buf[i] == '\b') {
            udriv_outb(port, buf[i]);
            udriv_outb(port, ' ');
            udriv_outb(port, buf[i]);
        }
		else {
			udriv_outb(port, buf[i]);
		}
		i++;
	}
}
