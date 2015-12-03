/* @file readline_server.c 
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

int main(int argc, char *argv[]) {

	if(argc > NUM_ARGS) {
		lprintf("Number of arguments to readline is incorrect");
		return ERR_FAILURE;
	}

    ipc_state_t* server_st;
	ipc_state_t* buf_st;
	if(argc == 1) {
		/* Launch the keyboard server */
		int server_tid = fork();
		if(server_tid < 0) {
			lprintf("Can't create keyboard_server");
			return ERR_FAILURE;
		}
		if(!server_tid) {
			char *args[] = {"keyboard_server", 0};
			if(exec("keyboard_server", args) < 0) {
				return ERR_FAILURE;
			}
		}

	    if (ipc_server_init(&server_st, UDR_READLINE_SERVER) < 0) {
        	return ERR_FAILURE;
	    }
    	if (ipc_server_init(&buf_st, KEYBOARD_READLINE_BUF_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}
	else if(strcmp(argv[1], "COM1") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM1_READLINE_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
    	if (ipc_server_init(&buf_st, COM1_READLINE_BUF_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}
	else if(strcmp(argv[1], "COM2") == 0) {
    	if (ipc_server_init(&server_st, UDR_COM2_READLINE_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
    	if (ipc_server_init(&buf_st, COM2_READLINE_BUF_SERVER) < 0) {
	        return ERR_FAILURE;
    	}
	}

    while (true) {
        driv_id_t sender;
		uint32_t msg_len;
		lprintf("Going to listen in readline server");
        if (ipc_server_recv(server_st, &sender, &msg_len, BUF_LEN, true) < 0) {
			lprintf("Failed in server_st recv");
            ipc_server_cancel(server_st);
            return ERR_FAILURE;
        }

		lprintf("Got a message in readline server");
		//Once a request comes, wait till you receive the line
		driv_id_t device;
		char buf[BUF_LEN];
		int len;
        if ((len = ipc_server_recv(buf_st, &device, &buf, 
									BUF_LEN, true)) < 0) {
			lprintf("Failed in buf_st recv");
            ipc_server_cancel(buf_st);
            return ERR_FAILURE;
        }

		lprintf("Received line of length %d", len);

		char *msg = (char *)malloc(msg_len);
		if(msg == NULL) {
			//TODO: What to do here?
		}
		memcpy(msg, buf, len);

		//Now that we have the line, send that back to the client
		ipc_server_send_msg(server_st, sender, msg, len);	
    }
}
