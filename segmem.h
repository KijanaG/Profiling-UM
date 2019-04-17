/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * UM
 *
 * segmem interface
 * 4/11/19
 *                                              
 */

#ifndef SEGMEM_H
#define SEGMEM_H
#include <stdio.h>
#include "uarray.h"
#include "seq.h"
#include "instruct.h"

UArray_T create_segment(int length);
void seg_mem_free(SegMem SegMem);
uint32_t map_seg_mem(SegMem SegMem, uint32_t size);
void unmap_seg(SegMem SegMem, uint32_t location);
void load_program(Seq_T seg_mem, uint32_t val);
void sstore(Seq_T seg_mem, uint32_t regA, uint32_t regB, uint32_t regC);
uint32_t sload(Seq_T seg_mem, uint32_t regB, uint32_t regC);

#endif