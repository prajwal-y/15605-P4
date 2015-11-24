/** @file fault_handlers_asm.S
 *
 *  asm file for fault handlers
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

.globl page_fault_handler
page_fault_handler:
	pusha						/* Save the general purpose registers */
	call page_fault_handler_c	/* Call the C handler for page fault */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl divide_error_handler
divide_error_handler:
	pusha						/* Save the general purpose registers */
	call divide_error_handler_c	/* Call the C handler for divide error */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl debug_exception_handler
debug_exception_handler:
	pusha							/* Save the general purpose registers */
	call debug_exception_handler_c	/* Call the C handler for debug exception */
	popa							/* Restore the registers */
	popl %ecx 						/* Pop the error code */
	iret

.globl non_maskable_interrupt_handler
non_maskable_interrupt_handler:
	pusha									/* Save the general purpose registers */
	call non_maskable_interrupt_handler_c	/* Call the C handler for NMI */
	popa									/* Restore the registers */
	popl %ecx 								/* Pop the error code */
	iret

.globl breakpoint_handler
breakpoint_handler:
	pusha						/* Save the general purpose registers */
	call breakpoint_handler_c	/* Call the C handler for breakpoint */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl overflow_handler
overflow_handler:
	pusha						/* Save the general purpose registers */
	call overflow_handler_c		/* Call the C handler for overflow */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl bound_range_handler
bound_range_handler:
	pusha						/* Save the general purpose registers */
	call bound_range_handler_c	/* Call the C handler for bound range */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl undefined_opcode_handler
undefined_opcode_handler:
	pusha							/* Save the general purpose registers */
	call undefined_opcode_handler_c	/* Call the C handler for undefined opcode */
	popa							/* Restore the registers */
	popl %ecx 						/* Pop the error code */
	iret

.globl no_math_coprocessor_handler
no_math_coprocessor_handler:
	pusha								/* Save the general purpose registers */
	call no_math_coprocessor_handler_c	/* Call the C handler for NMC */
	popa								/* Restore the registers */
	popl %ecx 							/* Pop the error code */
	iret

.globl cso_handler
cso_handler:
	pusha						/* Save the general purpose registers */
	call cso_handler_c			/* Call the C handler for page fault */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl invalid_tss_handler
invalid_tss_handler:
	pusha						/* Save the general purpose registers */
	call invalid_tss_handler_c	/* Call the C handler for invalid TSS */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl snp_handler
snp_handler:
	pusha				/* Save the general purpose registers */
	call snp_handler_c	/* Call the C handler for SNP */
	popa				/* Restore the registers */
	popl %ecx 			/* Pop the error code */
	iret

.globl ssf_handler
ssf_handler:
	pusha				/* Save the general purpose registers */
	call ssf_handler_c	/* Call the C handler for SSF */
	popa				/* Restore the registers */
	popl %ecx 			/* Pop the error code */
	iret

.globl gpf_handler
gpf_handler:
	pusha				/* Save the general purpose registers */
	call gpf_handler_c	/* Call the C handler for GPF */
	popa				/* Restore the registers */
	popl %ecx 			/* Pop the error code */
	iret

.globl math_fault_handler
math_fault_handler:
	pusha						/* Save the general purpose registers */
	call math_fault_handler_c	/* Call the C handler for math fault */
	popa						/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl alignment_check_handler 
alignment_check_handler:
	pusha							/* Save the general purpose registers */
	call alignment_check_handler_c	/* Call the C handler for alignment check */
	popa							/* Restore the registers */
	popl %ecx 					/* Pop the error code */
	iret

.globl machine_check_handler
machine_check_handler:
	pusha							/* Save the general purpose registers */
	call machine_check_handler_c	/* Call the C handler for MCE */
	popa							/* Restore the registers */
	popl %ecx 						/* Pop the error code */
	iret

.globl floating_point_exp_handler
floating_point_exp_handler:
	pusha								/* Save the general purpose registers */
	call floating_point_exp_handler_c	/* Call the C handler FPE */
	popa								/* Restore the registers */
	popl %ecx 							/* Pop the error code */
	iret
