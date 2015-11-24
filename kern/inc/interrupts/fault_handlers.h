/** @file fault_handlers.h
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef __FAULT_HANDLERS_H
#define __FAULT_HANDLERS_H
void tickback(unsigned int ticks);

void divide_error_handler_c();
void debug_exception_handler_c();
void non_maskable_interrupt_handler_c();
void breakpoint_handler_c();
void overflow_handler_c();
void bound_range_handler_c();
void undefined_opcode_handler_c();
void no_math_coprocessor_handler_c();
void cso_handler_c();
void invalid_tss_handler_c();
void snp_handler_c();
void ssf_handler_c();
void gpf_handler_c();
void page_fault_handler_c();
void math_fault_handler_c();
void alignment_check_handler_c();
void machine_check_handler_c();
void floating_point_exp_handler_c();

#endif /* __FAULT_HANDLERS_H */
