/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * UM
 *
 * instruct implementation
 * 4/11/19
 *                                              
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "uarray.h"
#include "seq.h"
#include "instruct.h"
#include "assert.h"
#include "segmem.h"
#include <bitpack.h>

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

typedef enum UM_opcode {
        CMOV = 0, SLOAD, SSTORE, ADD, MUL, DIV,
        NAND, HALT, MAP, UNMAP, OUTPUT, INPUT, LOADP, LOADV
} UM_opcode;

/* Init_program: Runs the whole UM program after taking in the 
   input file and the number of instructions from that file. */
void init_program(int num_instructs, char* input)
{
        UM UM         = create_UM(num_instructs);
        SegMem SegMem = create_SegMem(num_instructs);
        load_instructs(SegMem, input);
        execute(UM, SegMem);
        free_all(UM, SegMem);
}

/* creature_UM:  Returns a UM struct type after creating a new UM 
   with its registers that are currently initialized with 0 value */
UM create_UM()
{
        UM UM = malloc(sizeof(struct UM));
        assert(UM);        
        UM->prog_counter = 0;
        UM->registers = malloc(NUM_REG * sizeof(uint32_t));
        for(int i=0; i< NUM_REG; i++)
                UM->registers[i] = 0;
        return UM;
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

/* create_SegMem: Creates the segmented memory by initializing the 
   first segment based on the number of instructions taken from the input 
   file. */
SegMem create_SegMem(int num_instructs)
{
        SegMem SegMem   = malloc(sizeof(struct SegMem));
        SegMem->seg_mem = Seq_new(SEQ_HINT);
        Seq_addhi(SegMem->seg_mem, create_segment(num_instructs));
        SegMem->unmapped_segs = Seq_new(SEQ_HINT);
        return SegMem;
}

/* load_instructs: Store the instructions from the file into 
   the initial segment of memory. */
void load_instructs(SegMem SegMem, char *input)
{
        FILE* input_file = fopen(input, "rb");
        UArray_T curr_array = Seq_get(SegMem->seg_mem, SEG_ZERO);
        int length = UArray_length(curr_array);
        uint32_t *curr_elem;
        uint32_t instruction;
        for(int i = 0; i < length; i++) {
                instruction = 0;
                instruction = (uint32_t)Bitpack_newu(instruction,CHAR_SIZE,24, 
                                        fgetc(input_file));
                instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 
                                        16, fgetc(input_file));
                instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 8, 
                                        fgetc(input_file));
                instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 0, 
                                        fgetc(input_file));
                curr_elem   = (uint32_t *)UArray_at(curr_array, i);
                *curr_elem  = instruction;
        }
        fclose(input_file);
}

/* execute: Goes through the instruction from the segmented memory one
  by one and increases the program counter each time an instruction 
  is completed. */
void execute(UM UM, SegMem SegMem)
{
        while(true) {
                uint32_t curr_instruct = *(uint32_t *) UArray_at((UArray_T)
                                Seq_get(SegMem->seg_mem, SEG_ZERO), 
                                UM->prog_counter);
                run_instruct(UM, SegMem, curr_instruct, &UM->prog_counter);
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

        switch(opcode){
                case CMOV:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        if(*regC != 0)
                                *regA = *regB;
                        break;
                case SLOAD:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        *regA = sload(SegMem->seg_mem, *regB, *regC);
                        break;
                case SSTORE:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        sstore(SegMem->seg_mem, *regA, *regB, *regC);
                        break;
                case ADD:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        *regA = (*regB + *regC);
                        break;
                case MUL:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        *regA = (*regB * *regC);
                        break;
                case DIV:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        assert(*regC != 0);
                        *regA = (*regB / *regC);
                        break;
                case NAND:
                        A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        A_LSB);
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regA  = &((UM->registers)[A]);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        *regA = ~(*regB & *regC);
                        break;
                case HALT:
                        free_all(UM, SegMem);
                        exit(EXIT_SUCCESS);
                        break;
                case MAP:
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regB  = &((UM->registers)[B]);
                        regC  = &((UM->registers)[C]);
                        *regB = map_seg_mem(SegMem, *regC);
                        break;
                case UNMAP:
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regC = &((UM->registers)[C]);
                        unmap_seg(SegMem, *regC);
                        break;
                case OUTPUT:
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regC = &((UM->registers)[C]);
                        putchar(*regC);
                        break;
                case INPUT:
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regC = &((UM->registers)[C]);
                        get_input(regC);
                        break;
                case LOADP:
                        B     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        B_LSB);
                        C     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        C_LSB);
                        regB = &((UM->registers)[B]);
                        regC = &((UM->registers)[C]);
                        if(*regB != 0)
                                load_program(SegMem->seg_mem, *regB);
                        *prog_counter = (*regC - 1);
                        break;
                case LOADV:
                        load_value(UM, instruction);
                        break;
                default:
                        fprintf(stderr,"Found Improperly formatted opcode.\n");
                        exit(EXIT_FAILURE);
                        break;
        }       
}

/* get_input: If the opcode is 11, this function is called and 
   applies an input value into register C */
void get_input(uint32_t *regC)
{
        char input = getchar();
        if(input == EOF)
                *regC = ~0;
        else
                *regC = (uint32_t)input;
}
/* load_value: If the opcode is 13, this function is called and 
   loads a 25 bit value into register A*/
void load_value(UM UM ,uint32_t instruction)
{
        uint32_t A     = (uint32_t)Bitpack_getu(instruction, ID_LEN, 
                                                        OP_LSB - ID_LEN);
        uint32_t value = (uint32_t)Bitpack_getu(instruction, 
                                                VAL_LENGTH, VAL_LSB);
        uint32_t *regA = &((UM->registers)[A]);
        *regA = value;
}
