#include <lmm.h>
#include <simics.h>
#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    lmm_dump(&malloc_lmm);
    exit(0);
}
