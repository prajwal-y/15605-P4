/** @file fault_handler_asm.S
 *
 *  asm file for interrupt handlers
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#ifndef __FAULT_HANDLER_ASM_H
#define __FAULT_HANDLER_ASM_H

/** @brief The page fault handler
 *  
 *  This function calls the C page fault handler.
 *
 *  @return void
 */
void page_fault_handler();

/** @brief The divide error handler
 *  
 *  This function calls the C divide error handler
 *
 *  @return void
 */
void divide_error_handler();

/** @brief The debug exception handler
 *  
 *  This function calls the C debug exception handler
 *
 *  @return void
 */
void debug_exception_handler();

/** @brief The NMI handler
 *  
 *  This function calls the CNMI handler
 *
 *  @return void
 */
void non_maskable_interrupt_handler();

/** @brief The breakpoint handler
 *  
 *  This function calls the C breakpoint handler
 *
 *  @return void
 */
void breakpoint_handler();

/** @brief overflow handler
 *  
 *  This function calls the C overflow handler
 *
 *  @return void
 */
void overflow_handler();

/** @brief bound range handler
 *  
 *  This function calls the C bound range error handler
 *
 *  @return void
 */
void bound_range_handler();

/** @brief Undefined opcode handler
 *  
 *  This function calls the C undefined opcode handler
 *
 *  @return void
 */
void undefined_opcode_handler();

/** @brief The NMC handler
 *  
 *  This function calls the C NMC handler
 *
 *  @return void
 */
void no_math_coprocessor_handler();

/** @brief The CSO handler
 *  
 *  This function calls the C CSO handler
 *
 *  @return void
 */
void cso_handler();

/** @brief The TSS handler
 *  
 *  This function calls the C TSS handler
 *
 *  @return void
 */
void invalid_tss_handler();

/** @brief The SNP handler
 *  
 *  This function calls the C SNP handler
 *
 *  @return void
 */
void snp_handler();

/** @brief The stack segment fault handler
 *  
 *  This function calls the C SSF handler.
 *
 *  @return void
 */
void ssf_handler();

/** @brief The general protection fault handler
 *  
 *  This function calls the C GPF handler.
 *
 *  @return void
 */
void gpf_handler();

/** @brief The page fault handler
 *  
 *  This function calls the C page fault handler.
 *
 *  @return void
 */
void page_fault_handler();

/** @brief The math fault handler
 *  
 *  This function calls the C math fault handler
 *
 *  @return void
 */
void math_fault_handler();

/** @brief The alignment check handler
 *  
 *  This function calls the C alignment check handler
 *
 *  @return void
 */
void alignment_check_handler();

/** @brief The MCE handler
 *  
 *  This function calls the C MCE handler.
 *
 *  @return void
 */
void machine_check_handler();

/** @brief The FPE handler
 *  
 *  This function calls the C FPE handler.
 *
 *  @return void
 */
void floating_point_exp_handler();

#endif
