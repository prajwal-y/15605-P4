/** Functions to implement the keyboard server 
 *
 * @file keyboard_server.c
 * 
 * @author Rohit Upadhyaya (rjupadhy) 
 * @author Prajwal Yadapadithaya (pyadapad)
 **/

#include <stdio.h>
#include <string.h>
#include <key_circular_buffer.h>
#include <stdbool.h>
#include <syscall.h>
#include <ipc_client.h>
#include <errors.h>
#include <udriv_servers.h>
#include <keyhelp.h>

#define BUF_LEN 1024

const char back_space = '\b';
const char new_line = '\n';
const char space = ' ';

int main(int argc, char *argv[]) {

	if(udriv_register(UDR_KEYBOARD, KEYBOARD_PORT, 1) < 0) {
		return ERR_FAILURE;
	}

	driv_id_t driv_recv;
	message_t msg_recv;
	unsigned int msg_size;
    while (true) {
		if (udriv_wait(&driv_recv, &msg_recv, &msg_size) < 0) {
			break;
		}
		unsigned char in = (unsigned char)msg_recv;
		kh_type key = process_scancode(in);
		char c = space;
		if ((KH_HASDATA(key) != 0) && (KH_ISMAKE(key) == 0)) {
			c = KH_GETCHAR(key);
			if(c == back_space && !has_key()) {
				continue;
			}
			add_keystroke(c);
		} else {
			continue;
		}
        char pr_buf[1] = { c };
        print(1, pr_buf);
		if(c == new_line) {
			char buf[BUF_LEN];
			int num_char = get_nextline(buf, BUF_LEN);
			ipc_client_send_msg(KEYBOARD_READLINE_BUF_SERVER, 
									buf, num_char, NULL, 0);
		}
    }
	
	return ERR_FAILURE;
}
