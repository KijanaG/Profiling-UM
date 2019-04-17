/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * UM
 *
 * segmem implementation
 * 4/11/19
 *                                              
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <bitpack.h>
#include "seq.h"
#include "assert.h"
#include "uarray.h"
#include <stdint.h>
#include "segmem.h"

#define SEG_ZERO 0


/* create_segment: Initializes the first segment of 32 bit size based 
   on the length of the number of instructions given to the UM*/
UArray_T create_segment(int length){
	UArray_T segment = UArray_new(length, sizeof(uint32_t));
        uint32_t *curr_elem;
        for(int i=0;i<length;i++) {
                curr_elem = (uint32_t *)UArray_at(segment, i);
                *curr_elem = 0;
        }
	return segment;
}

/* create_segment: Frees all currently used IDs of segmented memory 
   and all the segments associated with does IDs*/
void seg_mem_free(SegMem SegMem)
{
        for(int i = 0; i < Seq_length(SegMem->seg_mem); i++){
                UArray_T temp = Seq_get(SegMem->seg_mem, i);
                if(temp != NULL)
                        UArray_free(&temp);
        }
        Seq_free(&(SegMem->unmapped_segs));
        Seq_free(&(SegMem->seg_mem));
        free(SegMem);
}

/* Gets array at given Segment ID and returns the value at the given offset
   of the segment ID. */
uint32_t sload(Seq_T seg_mem, uint32_t regB, uint32_t regC)
{
        UArray_T curr_seg = Seq_get(seg_mem, regB); 
        return *((uint32_t *)UArray_at(curr_seg, regC));
}

/* Takes value at given register and places it in the a given memory segment 
   at a given offset. */
void sstore(Seq_T seg_mem, uint32_t regA, uint32_t regB, uint32_t regC)
{
        uint32_t *curr_offset;
        UArray_T curr_seg = Seq_get(seg_mem, regA);
        curr_offset  = (uint32_t *)UArray_at(curr_seg, regB);
        *curr_offset = regC;
}

/* map_seg_mem: Based on if the opcode is 8, maps a new segment based 
  on the number of words in register C and makes an equivalently 
  segment in register B */
uint32_t map_seg_mem(SegMem SegMem, uint32_t size)
{
        UArray_T mapped_seg = create_segment(size);
        if(Seq_length(SegMem->unmapped_segs) > 0) {
                uint32_t location = (uint32_t)(uintptr_t)
                                     Seq_remlo(SegMem->unmapped_segs);
                Seq_put(SegMem->seg_mem, location, mapped_seg);
                return location;
        } else {
                Seq_addhi(SegMem->seg_mem, mapped_seg);
                return Seq_length(SegMem->seg_mem) - 1;
        }
}

/* unmap_seg: Based on if the opcode is 9, unmaps a segment at register
   C and then takes its ID so that it could be used again for future map calls.
    */
void unmap_seg(SegMem SegMem, uint32_t location)
{
        UArray_T unmap_seg = Seq_get(SegMem->seg_mem, location);
        UArray_free(&unmap_seg);
        Seq_put(SegMem->seg_mem, location, NULL);
        Seq_addhi(SegMem->unmapped_segs, (void *)(uintptr_t)location);
}

/* Load_program: Duplicates the segment connected to regB and puts it
   into the segment at ID 0 */
void load_program(Seq_T seg_mem, uint32_t val)
{
        uint32_t *curr_elem;
        uint32_t *new_elem;
        UArray_T curr_seg = Seq_get(seg_mem, val);
        int length        = UArray_length(curr_seg);
        UArray_T new_seg  = UArray_new(length, sizeof(uint32_t));
        for(int i=0; i<length; i++) {
                curr_elem = (uint32_t *)UArray_at(curr_seg, i);
                new_elem  = (uint32_t *)UArray_at(new_seg, i);
                *new_elem = *curr_elem;
        }
        UArray_T seg_zero = Seq_get(seg_mem, SEG_ZERO);
        UArray_free(&seg_zero);
        Seq_put(seg_mem, SEG_ZERO, NULL);
        Seq_put(seg_mem, SEG_ZERO, new_seg);
}
