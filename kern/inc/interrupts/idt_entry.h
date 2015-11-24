/** @file idt_entry.h
 *  @brief helper functions to manipulate and define the 
 *         IDT entry
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 */
#ifndef __IDT_ENTRY_H
#define __IDT_ENTRY_H

#define TRAP_GATE 0
#define INTERRUPT_GATE 1

#define KERNEL_DPL 0
#define USER_DPL 3

int add_idt_entry(void *handler, int interrupt_num, 
					int type, unsigned int dpl); 

#endif  /* __IDT_ENTRY_H */
