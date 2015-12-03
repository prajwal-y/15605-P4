/** Functions to implement the keyboard server 
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
#include <keyhelp.h>

#define BUF_LEN 1024

int main(int argc, char *argv[]) {

	if(udriv_register(UDR_KEYBOARD, KEYBOARD_PORT, 1) < 0) {
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
			ipc_client_send_str(KEYBOARD_READLINE_BUF_SERVER, 
								buf, NULL, 0);
		}
    }
	
	return ERR_FAILURE;
}
