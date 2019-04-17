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
    CMOV = 0,
    SLOAD,
    SSTORE,
    ADD,
    MUL,
    DIV,
    NAND,
    HALT,
    MAP,
    UNMAP,
    OUTPUT,
    INPUT,
    LOADP,
    LOADV
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

int get_size(char *input);
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
UArray_T create_segment(int length);
void seg_mem_free(SegMem SegMem);
uint32_t map_seg_mem(SegMem SegMem, uint32_t size);
void unmap_seg(SegMem SegMem, uint32_t location);
void load_program(Seq_T seg_mem, uint32_t val);
void sstore(Seq_T seg_mem, uint32_t regA, uint32_t regB, uint32_t regC);
uint32_t sload(Seq_T seg_mem, uint32_t regB, uint32_t regC);

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
        int num_instructs = get_size(argv[1]);
        UM UM = create_UM();
        SegMem SegMem = create_SegMem(num_instructs);
        load_instructs(SegMem, argv[1]);
        execute(UM, SegMem);
    }
    return EXIT_SUCCESS;
}

/* get_size: takes in an input file and returns the number lines 
   that exist in the file based on its size */
int get_size(char *input)
{
    struct stat st;
    if (stat(input, &st) != 0)
    {
        fprintf(stderr, "Incorrect File Size.\n");
        return EXIT_FAILURE;
    }
    assert(st.st_size % 4 == 0);
    return st.st_size / 4;
}

/* creature_UM:  Returns a UM struct type after creating a new UM 
   with its registers that are currently initialized with 0 value */
UM create_UM()
{
    UM UM = malloc(sizeof(struct UM));
    assert(UM);
    UM->prog_counter = 0;
    UM->registers = malloc(NUM_REG * sizeof(uint32_t));
    for (int i = 0; i < NUM_REG; i++)
        UM->registers[i] = 0;
    return UM;
}

/* create_SegMem: Creates the segmented memory by initializing the 
   first segment based on the number of instructions taken from the input 
   file. */
SegMem create_SegMem(int num_instructs)
{
    SegMem SegMem = malloc(sizeof(struct SegMem));
    SegMem->seg_mem = Seq_new(SEQ_HINT);
    Seq_addhi(SegMem->seg_mem, create_segment(num_instructs));
    SegMem->unmapped_segs = Seq_new(SEQ_HINT);
    return SegMem;
}

/* free_all: Frees all the values in the UM struct by going and 
   freeing any remaining values in the registers, as well as freeing all
   memory in the SegMem struct */
void free_all(UM UM, SegMem SegMem)
{
    uint32_t *temp = UM->registers;
    free((temp));
    free(UM);
    seg_mem_free(SegMem);
}

/* load_instructs: Store the instructions from the file into 
   the initial segment of memory. */
void load_instructs(SegMem SegMem, char *input)
{
    FILE *input_file = fopen(input, "rb");
    UArray_T curr_array = Seq_get(SegMem->seg_mem, SEG_ZERO);
    int length = UArray_length(curr_array);
    uint32_t *curr_elem;
    uint32_t instruction;
    for (int i = 0; i < length; i++)
    {
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
}

/* execute: Goes through the instruction from the segmented memory one
  by one and increases the program counter each time an instruction 
  is completed. */
void execute(UM UM, SegMem SegMem)
{
    while (true)
    {
        // uint32_t curr_instruct = *(uint32_t *) UArray_at((UArray_T)Seq_get(SegMem->seg_mem, SEG_ZERO), UM->prog_counter);
        run_instruct(UM, SegMem, *(uint32_t *)UArray_at((UArray_T)Seq_get(SegMem->seg_mem, SEG_ZERO), UM->prog_counter),
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
    uint32_t opcode = (uint32_t)Bitpack_getu(instruction, OP_LEN, OP_LSB);
    uint32_t A, B, C;
    uint32_t *regA = NULL;
    uint32_t *regB = NULL;
    uint32_t *regC = NULL;
    if (opcode <= 6) {
        if (opcode <= 3) {
            if (opcode <= 1) {
                if (opcode == 0) {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    if (*regC != 0)
                        *regA = *regB;
                } else {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    *regA = sload(SegMem->seg_mem, *regB, *regC);
                }
            } else {
                if (opcode == 2) {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    sstore(SegMem->seg_mem, *regA, *regB, *regC);
                } else {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    *regA = (*regB + *regC);
                }
            }
        } else {
            if (opcode <= 5) {
                if (opcode == 4) {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    *regA = (*regB * *regC);
                } else {
                    A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               A_LSB);
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regA = &((UM->registers)[A]);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    assert(*regC != 0);
                    *regA = (*regB / *regC);
                }
            } else {
                A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                           A_LSB);
                B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                           B_LSB);
                C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                           C_LSB);
                regA = &((UM->registers)[A]);
                regB = &((UM->registers)[B]);
                regC = &((UM->registers)[C]);
                *regA = ~(*regB & *regC);
            }
        }
    } else {
        if (opcode <= 10) {
            if (opcode <= 8) {
                if (opcode == 7) {
                    free_all(UM, SegMem);
                    exit(EXIT_SUCCESS);
                } else { //8
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN, B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN, C_LSB);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    *regB = map_seg_mem(SegMem, *regC);
                }
            }
            else {
                if (opcode == 9) {
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN, C_LSB);
                    regC = &((UM->registers)[C]);
                    unmap_seg(SegMem, *regC);
                } else { //10
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regC = &((UM->registers)[C]);
                    putchar(*regC);
                }
            }
        } else {
            if (opcode <= 12) {
                if (opcode == 11) {
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regC = &((UM->registers)[C]);
                    get_input(regC);
                } else { //12
                    B = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               B_LSB);
                    C = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                               C_LSB);
                    regB = &((UM->registers)[B]);
                    regC = &((UM->registers)[C]);
                    if (*regB != 0)
                        load_program(SegMem->seg_mem, *regB);
                    *prog_counter = (*regC - 1);
                }
            } else { //13
                load_value(UM, instruction);
            }
        }
    }
}

/* get_input: If the opcode is 11, this function is called and 
   applies an input value into register C */
void get_input(uint32_t *regC)
{
    char input = getchar();
    if (input == EOF)
        *regC = ~0;
    else
        *regC = (uint32_t)input;
}
/* load_value: If the opcode is 13, this function is called and 
   loads a 25 bit value into register A*/
void load_value(UM UM, uint32_t instruction)
{
    uint32_t A = (uint32_t)Bitpack_getu(instruction, ID_LEN,
                                        OP_LSB - ID_LEN);
    uint32_t value = (uint32_t)Bitpack_getu(instruction,
                                            VAL_LENGTH, VAL_LSB);
    uint32_t *regA = &((UM->registers)[A]);
    *regA = value;
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
void seg_mem_free(SegMem SegMem)
{
    for (int i = 0; i < Seq_length(SegMem->seg_mem); i++)
    {
        UArray_T temp = Seq_get(SegMem->seg_mem, i);
        if (temp != NULL)
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
    curr_offset = (uint32_t *)UArray_at(curr_seg, regB);
    *curr_offset = regC;
}

/* map_seg_mem: Based on if the opcode is 8, maps a new segment based 
  on the number of words in register C and makes an equivalently 
  segment in register B */
uint32_t map_seg_mem(SegMem SegMem, uint32_t size)
{
    UArray_T mapped_seg = create_segment(size);
    if (Seq_length(SegMem->unmapped_segs) > 0)
    {
        uint32_t location = (uint32_t)(uintptr_t)
            Seq_remlo(SegMem->unmapped_segs);
        Seq_put(SegMem->seg_mem, location, mapped_seg);
        return location;
    }
    else
    {
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
    int length = UArray_length(curr_seg);
    UArray_T new_seg = UArray_new(length, sizeof(uint32_t));
    for (int i = 0; i < length; i++)
    {
        curr_elem = (uint32_t *)UArray_at(curr_seg, i);
        new_elem = (uint32_t *)UArray_at(new_seg, i);
        *new_elem = *curr_elem;
    }
    UArray_T seg_zero = Seq_get(seg_mem, SEG_ZERO);
    UArray_free(&seg_zero);
    Seq_put(seg_mem, SEG_ZERO, NULL);
    Seq_put(seg_mem, SEG_ZERO, new_seg);
}