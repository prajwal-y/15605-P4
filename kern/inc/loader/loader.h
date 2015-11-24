/* The 15-410 kernel project
 *
 *     loader.h
 *
 * Structure definitions, #defines, and function prototypes
 * for the user process loader.
 */

#ifndef __LOADER_H
#define __LOADER_H
#include <elf_410.h>
#define PROG_PRESENT_VALID 0
#define PROG_ABSENT_INVALID 1

int load_program(simple_elf_t *se_hdr);

int getbytes(const char *filename, int offset, int size, char *buf);

int check_program(const char *prog_name);

#endif /* __LOADER_H */
