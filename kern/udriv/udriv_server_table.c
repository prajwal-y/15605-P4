/* @file udriv_server_table.h 
 *
 * @brief Contains the kernel-side lookup table listing well known
 * user servers for permissions
 *
 * @author Rohit Upadhyaya (rjupadhy)
 * @author Prajwal Yadapadithaya (pyadapad)
 */ 
#include <udriv_kern.h>
#include <udriv_registry.h>

const dev_spec_t server_table[] = {
    { 
        .id = UDR_COM1_PRINT_SERVER,
        .idt_slot = 32 + 4,
        .port_regions = { { 0x3f8, 8 } },
        .port_regions_cnt = 1,
        .mem_regions = { },
        .mem_regions_cnt = 0,
    },

    {
        .id = UDR_COM2_PRINT_SERVER,
        .idt_slot = 32 + 3,
        .port_regions = { { 0x2f8, 8 } },
        .port_regions_cnt = 1,
        .mem_regions = { },
        .mem_regions_cnt = 0,
    }
};

const int server_table_entries = (sizeof server_table) / (sizeof server_table[0]);
