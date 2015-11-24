/** @file asm.h
 *
 *  Contains the asm function declarations
 *
 */

#ifndef __ASM_H
#define __ASM_H

#include<stdint.h>

/** @brief Function to set the %cs register
 *
 *  @param val Value of the %cs register
 *
 *  @return void 
 */
void set_cs(uint16_t val);

/** @brief Function to set the %ss register
 *
 *  @param val Value of the %ss register
 *
 *  @return void 
 */
void set_ss(uint16_t val);

/** @brief Function to set the %ds register
 *
 *  @param val Value of the %ds register
 *
 *  @return void 
 */
void set_ds(uint16_t val);

/** @brief Function to set the %es register
 *
 *  @param val Value of the %es register
 *
 *  @return void 
 */
void set_es(uint16_t val);

/** @brief Function to set the %fs register
 *
 *  @param val Value of the %fs register
 *
 *  @return void 
 */
void set_fs(uint16_t val);

/** @brief Function to set the %gs register
 *
 *  @param val Value of the %gs register
 *
 *  @return void 
 */
void set_gs(uint16_t val);

void call_iret(uint32_t, uint32_t);

int get_cs();
int get_ds();
int get_ss();

/** @brief exit to userspace for new tasks
 */
void iret_fun();

/** @brief Function to change the kernel stack during
 * 			context switching.
 * 
 *  This is the core of context switching
 *
 *  @param esp esp of the new stack
 *  @param ebp ebp of the new stack
 *  @param addr_esp Address where the esp of the current stack
 *  				must be saved
 * 	@param addr_epb Address where the ebp of the current stack
 * 					must be saved
 *
 *  @return void
 */
void update_stack(uint32_t esp, uint32_t ebp, 
					uint32_t addr_esp, uint32_t addr_ebp);

/** @brief Function to change the kernel stack during
 * 			context switching.
 * 
 *  This is the core of context switching
 *
 *  @param esp esp of the new stack
 *  @param ebp ebp of the new stack
 *
 *  @return void
 */
void update_stack_single(uint32_t esp, uint32_t ebp);

void update_to_dead_thr_stack(uint32_t esp);

/** @brief Function to get the error code during a page fault
 *  
 *  This function uses the current stack pointer to get the 
 *  error code causing the page fault.
 *
 *  @return Returns the error code
 */
uint32_t get_err_code();

void *get_err_instr();

/** @brief Function to invalidate a particular page from TLB
 *
 *  @param addr Address to be invalidated from TLB
 *
 *  @return void
 */
void invalidate_tlb_page(void *addr);

#endif
