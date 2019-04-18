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

// int get_size(char *input);
UM create_UM();
SegMem create_SegMem(int num_instructs);
void load_instructs(SegMem SegMem, char *input);
void execute(UM UM, SegMem SegMem);
void run_instruct(UM UM, SegMem SegMem, uint32_t instruction,
                  uint32_t *prog_counter);
// void get_input(uint32_t *regC);
// void load_value(UM UM, uint32_t instruction);
// void unmap_seg(SegMem SegMem, uint32_t location);
// void free_all(UM UM, SegMem SegMem);
UArray_T create_segment(int length);
void seg_mem_free(SegMem SegMem);
// uint32_t map_seg_mem(SegMem SegMem, uint32_t size);
// void load_program(Seq_T seg_mem, uint32_t val);
// void sstore(Seq_T seg_mem, uint32_t regA, uint32_t regB, uint32_t regC);
// uint32_t sload(Seq_T seg_mem, uint32_t regB, uint32_t regC);

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
        Seq_addhi(SegMem->seg_mem, create_segment(num_instructs));
        SegMem->unmapped_segs = Seq_new(SEQ_HINT);
        // load_instructs(SegMem, argv[1]);
        FILE *input_file = fopen(argv[1], "rb");
        UArray_T curr_array = Seq_get(SegMem->seg_mem, SEG_ZERO);
        int length = UArray_length(curr_array);
        uint32_t *curr_elem;
        uint32_t instruction;
        for (int i = 0; i < length; i++) {
            instruction = 0;
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 24,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE,
                                                16, fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 8,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 0,
                                                fgetc(input_file));
            curr_elem = (uint32_t *)UArray_at(curr_array, i);
            *curr_elem = instruction;
        }
        fclose(input_file);
        execute(UM, SegMem);
    }
    return EXIT_SUCCESS;
}

/* free_all: Frees all the values in the UM struct by going and 
   freeing any remaining values in the registers, as well as freeing all
   memory in the SegMem struct */
// void free_all(UM UM, SegMem SegMem)
// {
//     uint32_t *temp = UM->registers;
//     free((temp));
//     free(UM);
//     seg_mem_free(SegMem);
// }


/* execute: Goes through the instruction from the segmented memory one
  by one and increases the program counter each time an instruction 
  is completed. */
void execute(UM UM, SegMem SegMem)
{
    while (true)
    {
        run_instruct(UM, SegMem, *(uint32_t *)UArray_at((UArray_T)Seq_get(SegMem->seg_mem, SEG_ZERO), 
        UM->prog_counter),
                     &UM->prog_counter);
        UM->prog_counter++;
    }
}

/* run_instruct: Parses an instruction for its opcode and then based on 
   the opcode of the instruction, the program possibly parses for registers 
   and completes one instruction per a call of this function */
void run_instruct(UM UM, SegMem SegMem, uint32_t instruction,
                  uint32_t *prog_counter)
{
    uint64_t word_0 = instruction;
    word_0 = word_0 << (32);
    word_0 = word_0 >> (60);
    uint32_t opcode = (uint32_t) word_0;
    uint32_t A, B, C;
    uint32_t *regA = NULL;
    uint32_t *regB = NULL;
    uint32_t *regC = NULL;
    if (opcode <= 6) {
        uint64_t word_1 = instruction; 
        word_1 = word_1 << (55);
        word_1 = word_1 >> (61);
        A = (uint32_t) word_1;
        uint64_t word_2 = instruction;
        word_2 = word_2 << (58);
        word_2 = word_2 >> (61);
        B = (uint32_t) word_2;
        uint64_t word_3 = instruction;
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
                    UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regB);
                    *regA = *((uint32_t *)UArray_at(curr_seg, *regC));
                }
            } else {
                if (opcode == 2) {
                    uint32_t *curr_offset;
                    UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regA);
                    curr_offset = (uint32_t *)UArray_at(curr_seg, *regB);
                    *curr_offset = *regC;
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
                        UArray_T temp = Seq_get(SegMem->seg_mem, i);
                        if (temp != NULL)
                            UArray_free(&temp);
                    }
                    Seq_free(&(SegMem->unmapped_segs));
                    Seq_free(&(SegMem->seg_mem));
                    free(SegMem);
                    exit(EXIT_SUCCESS);
                } else { //8
                    uint64_t word_2 = instruction;
                    word_2 = word_2 << (58);
                    word_2 = word_2 >> (61);
                    B = (uint32_t) word_2;
                    uint64_t word_3 = instruction;
                    word_3 = word_3 << (61);
                    word_3 = word_3 >> (61);
                    C = (uint32_t) word_3;
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    UArray_T segment = UArray_new(*regC, sizeof(uint32_t));
                    uint32_t *curr_elem;
                    for (int i = 0; i < (int)*regC; i++)
                    {
                        curr_elem = (uint32_t *)UArray_at(segment, i);
                        *curr_elem = 0;
                    }
                    UArray_T mapped_seg = segment;
                    if (Seq_length(SegMem->unmapped_segs) > 0) {
                        uint32_t location = (uint32_t)(uintptr_t)
                            Seq_remlo(SegMem->unmapped_segs);
                        Seq_put(SegMem->seg_mem, location, mapped_seg);
                        *regB = location;
                    } else {
                        Seq_addhi(SegMem->seg_mem, mapped_seg);
                        *regB = (Seq_length(SegMem->seg_mem) - 1);
                    }
                }
            }
            else {
                if (opcode == 9) {
                    uint64_t word_3 = instruction;
                    word_3 = word_3 << (61);
                    word_3 = word_3 >> (61);
                    C = (uint32_t) word_3;
                    regC = &((UM->registers)[C]);
                    // unmap_seg(SegMem, *regC);
                    UArray_T unmap_seg = Seq_get(SegMem->seg_mem, *regC);
                    UArray_free(&unmap_seg);
                    Seq_put(SegMem->seg_mem, *regC, NULL);
                    Seq_addhi(SegMem->unmapped_segs, (void *)(uintptr_t)*regC);
                } else { //10
                    uint64_t word_3 = instruction;
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
                    uint64_t word_3 = instruction;
                    word_3 = word_3 << (61);
                    word_3 = word_3 >> (61);
                    C = (uint32_t) word_3;
                    regC = &((UM->registers)[C]);
                    // get_input(regC);
                    char input = getchar();
                    if (input == EOF)
                        *regC = ~0;
                    else
                        *regC = (uint32_t)input;
                } else { //12
                    uint64_t word_2 = instruction;
                    word_2 = word_2 << (58);
                    word_2 = word_2 >> (61);
                    B = (uint32_t) word_2;
                    uint64_t word_3 = instruction;
                    word_3 = word_3 << (61);
                    word_3 = word_3 >> (61);
                    C = (uint32_t) word_3;
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    if (*regB != 0) {
                            // load_program(SegMem->seg_mem, *regB);
                            uint32_t *curr_elem;
                            uint32_t *new_elem;
                            UArray_T curr_seg = Seq_get(SegMem->seg_mem, *regB);
                            int length = UArray_length(curr_seg);
                            UArray_T new_seg = UArray_new(length, sizeof(uint32_t));
                            for (int i = 0; i < length; i++) {
                                curr_elem = (uint32_t *)UArray_at(curr_seg, i);
                                new_elem = (uint32_t *)UArray_at(new_seg, i);
                                *new_elem = *curr_elem;
                            }
                            UArray_T seg_zero = Seq_get(SegMem->seg_mem, SEG_ZERO);
                            UArray_free(&seg_zero);
                            Seq_put(SegMem->seg_mem, SEG_ZERO, NULL);
                            Seq_put(SegMem->seg_mem, SEG_ZERO, new_seg);
                    }
                    *prog_counter = (*regC - 1);
                }
            } else { //13
                // load_value(UM, instruction);
                    uint64_t word_1 = instruction;
                    word_1 = word_1 << (36);
                    word_1 = word_1 >> (61);
                    A = (uint32_t) word_1;
                    uint64_t word_4 = instruction;
                    word_4 = word_4 << (39);
                    word_4 = word_4 >> (39);
                    uint32_t value = (uint32_t) word_4;
                    uint32_t *regA = &((UM->registers)[A]);
                    *regA = value;
            }
        }
    }
}


/* create_segment: Initializes the first segment of 32 bit size based 
   on the length of the number of instructions given to the UM*/
UArray_T create_segment(int length)
{
    UArray_T segment = UArray_new(length, sizeof(uint32_t));
    uint32_t *curr_elem;
    for (int i = 0; i < length; i++)
    {
        curr_elem = (uint32_t *)UArray_at(segment, i);
        *curr_elem = 0;
    }
    return segment;
}

/* create_segment: Frees all currently used IDs of segmented memory 
   and all the segments associated with does IDs*/
// void seg_mem_free(SegMem SegMem)
// {
//     for (int i = 0; i < Seq_length(SegMem->seg_mem); i++)
//     {
//         UArray_T temp = Seq_get(SegMem->seg_mem, i);
//         if (temp != NULL)
//             UArray_free(&temp);
//     }
//     Seq_free(&(SegMem->unmapped_segs));
//     Seq_free(&(SegMem->seg_mem));
//     free(SegMem);
// }
