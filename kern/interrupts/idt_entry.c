/** @file idt_entry.c
 *  @brief Implementation of functions to read and 
 *         manipulate IDT entries
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadpad)
 */

#include <interrupts/idt_entry.h>
#include <asm.h>
#include <seg.h>
#include <common/errors.h>
#include <string/string.h>

#define TRAP_GATE_FLAGS 15
#define INTERRUPT_GATE_FLAGS 14

#define ZEROES 0
#define PRESENT 1

/** @brief a struct to store the fields of an IDT entry
 *         in a compact fashion. 
 */
typedef struct {
    unsigned offset_1:16;
    unsigned seg_selector:16;
    unsigned reserved:5;
    unsigned zeroes:3;
    unsigned flags:5;
    unsigned dpl:2;
    unsigned present:1;
    unsigned offset_2:16;
} idt_entry;

static void get_default_trap_idt(idt_entry *entry, unsigned int);
static void get_default_interrupt_idt(idt_entry *entry, unsigned int);

/** @brief function to add a entry in the IDT table
 *
 *  The function calls the idt_base() function to retrieve the 
 *  base of the IDT and uses the interrupt_num to figure out the 
 *  right memory location to copy the interrupt handler to
 *
 *  @param handler the address of the interrupt handler
 *  @param interrupt_num the interrupt for which we wish to install an
 *                       interrupt handler
 *  @param type the type of interrupt descriptor to be added 
 *              (TRAP_GATE/INTERRUPT_GATE)
 *  @param dpl the privilege level of the descriptor
 *  @return int 0 on success. -ve integer on failure
 */
int add_idt_entry(void *handler, int interrupt_num, int type, unsigned int dpl) {
    if (type != TRAP_GATE && type != INTERRUPT_GATE) {
        return ERR_FAILURE;
    }
    void *idt_base_addr = idt_base();
    idt_entry entry;
    if (type == TRAP_GATE) {
        get_default_trap_idt(&entry, dpl);
    } 
    else if (type == INTERRUPT_GATE) {
        get_default_interrupt_idt(&entry, dpl);
    }

    entry.offset_1 = (int)handler & 0xffff;
    entry.offset_2 = ((int)handler >> 16) & 0xffff;
    memcpy((char *)idt_base_addr + (8 * interrupt_num), &entry, 
            sizeof(idt_entry));

	return 0;
}

/** @brief function to return a pointer to an idt_entry struct 
 *         with the default values filled in for a trap gate
 *
 *  @param entry the idt entry to be filled up
 *  @param dpl the descriptor privilege level
 *  @return void
 */
void get_default_trap_idt(idt_entry *entry, unsigned int dpl) {
    entry->seg_selector = SEGSEL_KERNEL_CS;
    entry->zeroes = ZEROES;
    entry->flags = TRAP_GATE_FLAGS;
    entry->dpl = dpl;
    entry->present = PRESENT;
}

/** @brief function to return a pointer to an idt_entry struct 
 *         with the default values filled ini for an interrupt gate
 *
 *  @param entry the idt entry to be filled up
 *  @param dpl the descriptor privilege level
 *  @return void
 */
void get_default_interrupt_idt(idt_entry *entry, unsigned int dpl) {
    entry->seg_selector = SEGSEL_KERNEL_CS;
    entry->zeroes = ZEROES;
    entry->flags = INTERRUPT_GATE_FLAGS;
    entry->dpl = dpl;
    entry->present = PRESENT;
}
