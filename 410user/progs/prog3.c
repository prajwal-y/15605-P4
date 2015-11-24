/** @file 410user/progs/idle.c
 *  @author ?
 *  @brief Idle program.
 *  @public yes
 *  @for p2 p3
 *  @covers
 *  @status done
 */
#include <simics.h>
#include <syscall.h>
#include <stdio.h>
#include <stdlib.h>

void recursive(int num) {
	recursive(num+1);
}

int main()
{
	recursive(1);
	exit(0);
}
