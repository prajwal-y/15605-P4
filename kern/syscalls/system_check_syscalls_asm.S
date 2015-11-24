/** @file system_check_syscalls_asm.S
 *  
 *  handlers for system check system calls
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include<simics.h>

.globl memory_check_handler
memory_check_handler:
    pusha
    call memory_check_handler_c
    popa
    iret
