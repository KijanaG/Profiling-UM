/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * UM
 *
 * UM’s main file
 * 4/11/19
 *                                              
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <bitpack.h>
#include "seq.h"
#include "assert.h"
#include <stdint.h>
#include "segmem.h"
#include "uarray.h"
#include "instruct.h"


int get_size(char *input);

int main(int argc, char *argv[])
{
        if (argc > 2) {
                fprintf(stderr, "Too many arguments.\n");
                return EXIT_FAILURE;
        } else if (argc < 2) {
                fprintf(stderr, "Too few arguments.\n");
                return EXIT_FAILURE;
        } else {
                int num_instructs = get_size(argv[1]);
                init_program(num_instructs, argv[1]);
        }
        return EXIT_SUCCESS;
}

/* get_size: takes in an input file and returns the number lines 
   that exist in the file based on its size */
int get_size(char *input)
{
        struct stat st;
        if (stat(input, &st) != 0) {
                fprintf(stderr, "Incorrect File Size.\n");
                return EXIT_FAILURE;
        }
        assert(st.st_size % 4 == 0);
        return st.st_size / 4;
}