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
#include "bitpack.h"
#include "seq.h"
#include "assert.h"
#include <stdint.h>
#include "uarray.h"

#define CHAR_SIZE 8
#define NUM_REG 8
#define SEQ_HINT 10
#define SEG_ZERO 0
#define VAL_LENGTH 25
#define VAL_LSB 0
#define ID_LEN 3
#define A_LSB 6
#define B_LSB 3
#define C_LSB 0
#define OP_LEN 4
#define OP_LSB 28

typedef enum UM_opcode
{
    CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV, NAND, HALT, MAP, UNMAP,
    OUTPUT, INPUT, LOADP, LOADV
} UM_opcode;

typedef struct UM
{
    uint32_t prog_counter;
    uint32_t *registers;
} * UM;

typedef struct SegMem
{
    Seq_T seg_mem;
    Seq_T unmapped_segs;
} * SegMem;

typedef struct segment
{
    int length;
    uint32_t seg[];
} * segment;


int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        fprintf(stderr, "Too many arguments.\n");
        return EXIT_FAILURE;
    }
    else if (argc < 2)
    {
        fprintf(stderr, "Too few arguments.\n");
        return EXIT_FAILURE;
    }
    else
    {
        struct stat st;
        if (stat(argv[1], &st) != 0) {
            fprintf(stderr, "Incorrect File Size.\n");
            return EXIT_FAILURE;
        }
        int num_instructs = st.st_size / 4;
        UM UM = malloc(sizeof(struct UM));
        UM->prog_counter = 0;
        UM->registers = malloc(NUM_REG * sizeof(uint32_t));
        for (int i = 0; i < NUM_REG; i++)
            UM->registers[i] = 0;
        SegMem SegMem = malloc(sizeof(struct SegMem));
        SegMem->seg_mem = Seq_new(SEQ_HINT);
        // Seq_addhi(SegMem->seg_mem, create_segment(num_instructs));

        segment seg_zero = malloc(num_instructs * sizeof(uint32_t)+sizeof(int));
        for(int i=0; i< num_instructs; i++) {
            seg_zero->seg[i] = 0;
        }
        seg_zero->length = num_instructs;
        Seq_addhi(SegMem->seg_mem, seg_zero);

        SegMem->unmapped_segs = Seq_new(SEQ_HINT);
        FILE *input_file = fopen(argv[1], "rb");
        // UArray_T curr_array = Seq_get(SegMem->seg_mem, SEG_ZERO);
        // int length = UArray_length(curr_array);
        // uint32_t *curr_elem;
        uint32_t instruction;
        for (int i = 0; i < num_instructs; i++) {
            instruction = 0;
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 24,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE,
                                                16, fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 8,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 0,
                                                fgetc(input_file));
            // curr_elem = (uint32_t *)UArray_at(curr_array, i);
            // *curr_elem = instruction;
            seg_zero->seg[i] = instruction;
        }
        fclose(input_file);
        // UArray_T seg_zero = (UArray_T)Seq_get(SegMem->seg_mem, SEG_ZERO);
        while (true) 
        {
            // run_instruct(UM, SegMem, seg_zerooo[UM->prog_counter],
            // seg_zerooo, &UM->prog_counter);
            uint32_t curr_instruct = seg_zero->seg[UM->prog_counter];
            uint64_t word_0 = curr_instruct;
            word_0 = word_0 << (32);
            word_0 = word_0 >> (60);
            uint32_t opcode = (uint32_t) word_0;
            uint32_t A, B, C;
            uint32_t *regA = NULL;
            uint32_t *regB = NULL;
            uint32_t *regC = NULL;
            if (opcode <= 6) {
                uint64_t word_1 = curr_instruct; 
                word_1 = word_1 << (55);
                word_1 = word_1 >> (61);
                A = (uint32_t) word_1;
                uint64_t word_2 = curr_instruct;
                word_2 = word_2 << (58);
                word_2 = word_2 >> (61);
                B = (uint32_t) word_2;
                uint64_t word_3 = curr_instruct;
                word_3 = word_3 << (61);
                word_3 = word_3 >> (61);
                C = (uint32_t) word_3;
                regA = &((UM->registers)[A]);
                regB = &((UM->registers)[B]);
                regC = &((UM->registers)[C]);
                if (opcode <= 3) {
                    if (opcode <= 1) {
                        if (opcode == 0) {
                            if (*regC != 0)
                                *regA = *regB;
                        } else {
                            // UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regB);
                            segment curr_seg = Seq_get(SegMem->seg_mem, *regB);
                            *regA = curr_seg->seg[*regC];
                            // *regA = *((uint32_t *)UArray_at(curr_seg, *regC));
                        }
                    } else {
                        if (opcode == 2) {
                            // uint32_t *curr_offset;
                            // UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regA);
                            segment curr_seg = Seq_get(SegMem->seg_mem, *regA);
                            // curr_offset = (uint32_t *)UArray_at(curr_seg, *regB);
                            // *curr_offset = *regC;
                            curr_seg->seg[*regB] = *regC;
                        } else {
                            *regA = (*regB + *regC);
                        }
                    }
                } else {
                    if (opcode <= 5) {
                        if (opcode == 4) {
                            *regA = (*regB * *regC);
                        } else {
                            *regA = (*regB / *regC);
                        }
                    } else {
                        *regA = ~(*regB & *regC);
                    }
                }
            } else {
                if (opcode <= 10) {
                    if (opcode <= 8) {
                        if (opcode == 7) {
                            uint32_t *temp = UM->registers;
                            free((temp));
                            free(UM);
                            for (int i = 0; i < Seq_length(SegMem->seg_mem); i++) {
                                // UArray_T temp = Seq_get(SegMem->seg_mem, i);
                                segment temp = Seq_get(SegMem->seg_mem, i);
                                if (temp != NULL)
                                    free(temp);
                            }
                            Seq_free(&(SegMem->unmapped_segs));
                            Seq_free(&(SegMem->seg_mem));
                            free(SegMem);
                            exit(EXIT_SUCCESS);
                        } else { //8
                            uint64_t word_2 = curr_instruct;
                            word_2 = word_2 << (58);
                            word_2 = word_2 >> (61);
                            B = (uint32_t) word_2;
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            regB = &((UM->registers)[B]);
                            regC = &((UM->registers)[C]);
                            // UArray_T segment = UArray_new(*regC, sizeof(uint32_t));
                            segment segment = malloc(*regC * sizeof(uint32_t) + sizeof(int));
                            for(int i=0; i < (int)*regC; i++) {
                                segment->seg[i] = 0;
                            }
                            segment->length = *regC;
                            // uint32_t *curr_elem;
                            // for (int i = 0; i < (int)*regC; i++)
                            // {
                            //     curr_elem = (uint32_t *)UArray_at(segment, i);
                            //     *curr_elem = 0;
                            // }
                            // UArray_T mapped_seg = segment;
                            if (Seq_length(SegMem->unmapped_segs) > 0) {
                                uint32_t location = (uint32_t)(uintptr_t)
                                    Seq_remlo(SegMem->unmapped_segs);
                                // Seq_put(SegMem->seg_mem, location, mapped_seg);
                                Seq_put(SegMem->seg_mem, location, segment);
                                *regB = location;
                            } else {
                                // Seq_addhi(SegMem->seg_mem, mapped_seg);
                                Seq_addhi(SegMem->seg_mem, segment);
                                *regB = (Seq_length(SegMem->seg_mem) - 1);
                            }
                        }
                    }
                    else {
                        if (opcode == 9) {
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            regC = &((UM->registers)[C]);
                            // UArray_T unmap_seg = Seq_get(SegMem->seg_mem, *regC);
                            segment unmap_seg = Seq_get(SegMem->seg_mem, *regC);
                            free(unmap_seg);
                            Seq_put(SegMem->seg_mem, *regC, NULL);
                            Seq_addhi(SegMem->unmapped_segs, (void *)(uintptr_t)*regC);
                        } else { //10
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            regC = &((UM->registers)[C]);
                            putchar(*regC);
                        }
                    }
                } else {
                    if (opcode <= 12) {
                        if (opcode == 11) {
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            regC = &((UM->registers)[C]);
                            char input = getchar();
                            if (input == EOF)
                                *regC = ~0;
                            else
                                *regC = (uint32_t)input;
                        } else { //12
                            uint64_t word_2 = curr_instruct;
                            word_2 = word_2 << (58);
                            word_2 = word_2 >> (61);
                            B = (uint32_t) word_2;
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            regB = &((UM->registers)[B]);
                            regC = &((UM->registers)[C]);
                            if (*regB != 0) {
                                    // uint32_t *curr_elem;
                                    // uint32_t *new_elem;
                                    // UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regB);
                                    segment curr_seg = Seq_get(SegMem->seg_mem, *regB);
                                    // int length = UArray_length(curr_seg);
                                    // UArray_T new_seg = UArray_new(length, sizeof(uint32_t));
                                    segment new_seg = malloc(curr_seg->length * sizeof(uint32_t) + sizeof(int));
                                    for(int i=0; i< num_instructs; i++) {
                                        new_seg->seg[i] = 0;
                                    }
                                    new_seg->length = curr_seg->length;
                                    for (int i = 0; i < new_seg->length; i++) {
                                        // curr_elem = (uint32_t *)UArray_at(curr_seg, i);
                                        // new_elem = (uint32_t *)UArray_at(new_seg, i);
                                        // *new_elem = *curr_elem;
                                        new_seg->seg[i] = curr_seg->seg[i];
                                    }
                                    // UArray_free(seg_zero);
                                    free(seg_zero);
                                    Seq_put(SegMem->seg_mem, SEG_ZERO, NULL);
                                    Seq_put(SegMem->seg_mem, SEG_ZERO, new_seg);
                                    seg_zero = new_seg;
                            }
                            UM->prog_counter = (*regC - 1);
                        }
                    } else { //13
                            uint64_t word_1 = curr_instruct;
                            word_1 = word_1 << (36);
                            word_1 = word_1 >> (61);
                            A = (uint32_t) word_1;
                            uint64_t word_4 = curr_instruct;
                            word_4 = word_4 << (39);
                            word_4 = word_4 >> (39);
                            uint32_t value = (uint32_t) word_4;
                            uint32_t *regA = &((UM->registers)[A]);
                            *regA = value;
                    }
                }
            }
            UM->prog_counter++;
        } 
    }
    return EXIT_SUCCESS;
}

/* run_instruct: Parses an instruction for its opcode and then based on 
   the opcode of the instruction, the program possibly parses for registers 
   and completes one instruction per a call of this function */
// void run_instruct(UM UM, SegMem SegMem, uint32_t instruction, uint32_t *seg_zero,
//                   uint32_t *prog_counter)
// {

// }

