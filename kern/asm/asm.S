/** @file asm.S
 *
 *  Implementations of the set segment selector 
 *  register functions defined in asm.h
 *  
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef SEGSEL_USER_CS
#define SEGSEL_USER_CS 0x23
#endif

#ifndef SEGSEL_USER_DS
#define SEGSEL_USER_DS 0x2b
#endif

#ifndef STACK_START
#define STACK_START 0xc0000000
#endif

.globl get_cs
get_cs:
	movl %cs, %eax
	ret

.globl get_ds
get_ds:
	movl %ds, %eax
	ret

.globl get_ss
get_ss:
	movl %ss, %eax
	ret

.globl set_cs
set_cs:
    movl 4(%esp), %ecx /*Get the parameter*/
    mov %ecx, %cs  /* Set the code segment selector*/
    ret

.globl set_ss
set_ss:
    movl 4(%esp), %ecx
    mov %ecx, %ss /*Set the stack segment selector*/
    ret

.globl set_ds
set_ds:
    movl 4(%esp), %ecx
    mov %ecx, %ds /*Set the data segment selector*/
    ret

.globl set_es
set_es:
    movl 4(%esp), %ecx
    mov %ecx, %es
    ret

.globl set_fs
set_fs:
    movl 4(%esp), %ecx
    mov %ecx, %fs
    ret

.globl set_gs
set_gs:
    movl 4(%esp), %ecx
    mov %ecx, %gs
    ret

.globl call_iret
call_iret:
	movl %esp, %ecx
	mov $SEGSEL_USER_DS, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	
	pushl $SEGSEL_USER_DS		/*SS*/
	pushl $STACK_START			/*ESP*/
	pushl 4(%ecx)				/*EFLAGS*/
	pushl $SEGSEL_USER_CS		/*CS*/
	pushl 8(%ecx)				/*EIP*/
	iret

.globl iret_fun
iret_fun:
	sti
	popa
	movl $0, %eax
	iret

.globl update_stack
update_stack:
	movl 16(%esp), %ecx	/* Address where current ebp must be stored */
	movl %ebp, (%ecx)	/* Save the current ebp */
	movl 12(%esp), %ecx	/* Address where current esp must be stored */
	movl %esp, (%ecx)	/* Save the current esp */
	movl 8(%esp), %ecx	/* The new ebp */
	movl %ecx, %ebp		/* Set the new ebp */
	movl 4(%esp), %ecx	/* The new esp */
	movl %ecx, %esp		/* Set the new esp */
	ret

.globl update_stack_single
update_stack_single:
	movl 8(%esp), %ecx	/* The new ebp */
	movl %ecx, %ebp		/* Set the new ebp */
	movl 4(%esp), %ecx	/* The new esp */
	movl %ecx, %esp		/* Set the new esp */
	ret

.globl update_to_dead_thr_stack
update_to_dead_thr_stack:
	movl 4(%esp), %ecx	/* The new esp */
	subl $4, %ecx		/* The new value of esp */
	movl (%esp), %edx	/* Move the return address to a register */
	movl %edx, (%ecx)	/* Update the return address in the new stack */
	movl %ecx, %esp		/* Set the new esp */
	movl %ecx, %ebp		/* Set the new ebp */
	ret

.globl get_err_code
get_err_code:
	movl 4(%esp), %eax	/* Get the error code from the stack pointer*/
	ret

.globl get_err_instr
get_err_instr:
	movl 8(%esp), %eax	/* Get the error instr from the stack pointer*/
	ret

.globl invalidate_tlb_page
invalidate_tlb_page:
	invlpg 4(%esp)
	ret
