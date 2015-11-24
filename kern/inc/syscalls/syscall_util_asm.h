/** @file syscall_util_asm.h
 *
 *  Helper asm functions
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */
#ifndef __SYSCALL_UTIL_ASM_H
#define __SYSCALL_UTIL_ASM_H

#define SAVE_REGS \
		pushl %ecx; \
		pushl %eax; \
    	pushl %edx; \
    	pushl %ebx; \
    	pushl %esp; \
    	pushl %ebp; \
    	pushl %edi; \
    	pushl %esi;

#define RESTORE_REGS \
		popl %esi; \
	    popl %edi; \
    	popl %ebp; \
	    popl %esp; \
	    popl %ebx; \
	    popl %edx; \
	    popl %ecx; \
		popl %ecx;

#endif /* __SYSCALL_UTIL_ASM_H */
