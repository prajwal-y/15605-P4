/** @file mmap_test.c
 *
 *  Test file for mmap
 *
 *  @author Rohit Upadhyaya (rjupadhy)
 *  @author Prajwal Yadapadithaya (pyadapad)
 */

#include <stdio.h>
#include <errors.h>
#include <syscall.h>
#include <simics.h>
#include <udriv_registry.h>

#define VIDEO_MEM_ADDR 0xb8000
#define VIDEO_MEM_LEN 0x1000

#define MMAP_ADDR 0xb000000

#define CONSOLE_HEIGHT 25
#define CONSOLE_WIDTH 80

static void print_char(char ch, int row, int col, int color);

int main() {

	/* Register as console driver */
	if(udriv_register(UDR_CONSOLE, 0, 0) < 0) {
		lprintf("Registring as console driver failed");
		return ERR_FAILURE;
	}
	
	if(udriv_mmap((void *)VIDEO_MEM_ADDR, (void *)MMAP_ADDR, 
					VIDEO_MEM_LEN) < 0) {
		lprintf("mmap failed");
		return ERR_FAILURE;
	}

	/* Write to console memory */
	print_char('t', 10, 12, 0xF);
	print_char('e', 10, 13, 0xF);
	print_char('s', 10, 14, 0xF);
	print_char('t', 10, 15, 0xF);
	
	return 0;
}

/** @brief Prints a character to the console
 *
 *  @param ch Character to be printed
 *  @param row Row
 *  @param col Column
 *  @param color Color of the character
 *
 *  @return void
 */
void print_char(char ch, int row, int col, int color) {
	*(char *)(MMAP_ADDR + 2*(row * CONSOLE_WIDTH + col)) = ch;
	*(char *)(MMAP_ADDR + 2*(row * CONSOLE_WIDTH + col) + 1) = color;
}
