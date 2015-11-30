/** @file print_server.c
 * 
 * @author Rohit Upadhyaya (rjupadhy) 
 * @author Prajwal Yadapadithaya (pyadapad)
 **/

#include <stdio.h>
#include <stdbool.h>
#include <syscall.h>
#include <rand.h>
#include <ipc_server.h>

#define BUF_LEN 1024

int main() {

    int pid;
    if ((pid = fork()) != 0) {
        if (pid < 0) {
            printf("joke server could not be started\n");
            return -1;
        } else {
            printf("joke server started on pid %d\n", pid);
            return 0;
        }
    }    

    ipc_state_t* server_st;
    if (ipc_server_init(&server_st, UDR_JOKE_SERVER) < 0) {
        printf("could not register for joke server, exiting...\n");
        return -1;
    }

    while (true) {
        // Receive a request for a joke.
        driv_id_t sender;
        if (ipc_server_recv(server_st, &sender, &req, sizeof(req), true) < 0) {
            printf("could not receive request, exiting...\n");
            ipc_server_cancel(server_st);
            return -1;
        }

        ipc_server_send_str(server_st, sender, jokes[genrand() % NJOKES]);
    }
}
