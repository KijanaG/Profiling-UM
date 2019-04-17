/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * UM
 *
 * instruct interface
 * 4/11/19
 *                                              
 */

#ifndef INSTRUCT_H
#define INSTRUCT_H
#include <stdio.h>
#include "uarray.h"

typedef struct UM
{
        uint32_t prog_counter;
        uint32_t *registers;
} *UM;

typedef struct SegMem
{
        Seq_T seg_mem;
        Seq_T unmapped_segs;
} *SegMem;

void init_program(int num_instructs, char *input);
UM create_UM();
SegMem create_SegMem(int num_instructs);
void load_instructs(SegMem SegMem, char *input);
void execute(UM UM, SegMem SegMem);
void run_instruct(UM UM, SegMem SegMem, uint32_t instruction, 
                                        uint32_t *prog_counter);
void get_input(uint32_t *regC);
void load_value(UM UM, uint32_t instruction);
uint32_t map_seg_mem(SegMem SegMem, uint32_t size);
void unmap_seg(SegMem SegMem, uint32_t location);
void free_all(UM UM, SegMem SegMem);
#endif