/* 
 * KJ Garrett (kgarre01) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * Profiling 
 *
 * Profiling file
 * 4/11/19
 *                                              
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdint.h>

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

typedef struct segment
{
    int length;
    uint32_t seg[];
} * segment;

/* Purpose: Imported from Bitpack.c to speed up performance with static
            inline. */
static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, 
                                                unsigned lsb, uint64_t value)
{
        uint64_t bit_mask = ~0;
        bit_mask = bit_mask << (64 -  (width +lsb));
        bit_mask = bit_mask >> (64 - width);
        bit_mask = bit_mask << lsb;
        bit_mask = ~bit_mask;
        value = value << lsb;
        word = word & bit_mask;
        word = word | value;
        return word;
}

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

        uint32_t prog_counter = 0;
        uint32_t registers[8] = {0,0,0,0,0,0,0,0};
        int i;
            
        segment seg_zero = malloc(num_instructs * 
                                                sizeof(uint32_t)+sizeof(int));
        for(i=0; i< num_instructs; i++)
            seg_zero->seg[i] = 0;
    
        seg_zero->length = num_instructs;
        segment *STACK = NULL;
        uint32_t *QUEUE = NULL;
        STACK = calloc(num_instructs, sizeof(segment));
        STACK[0] = seg_zero;
        int segment_length = num_instructs;
        int segment_back = 1;
        QUEUE = malloc(100 * sizeof(uint32_t));
        int queue_len = 100;
        int queue_front = 0;
        int queue_back = 0;

        FILE *input_file = fopen(argv[1], "rb");

        uint32_t instruction;
        for (i = 0; i < num_instructs; i++) {
            instruction = 0;
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 24,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE,
                                                16, fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 8,
                                                fgetc(input_file));
            instruction = (uint32_t)Bitpack_newu(instruction, CHAR_SIZE, 0,
                                                fgetc(input_file));
            seg_zero->seg[i] = instruction;
        }
        fclose(input_file);
        segment *new_stack = NULL;
        while (1) 
        {
            uint32_t curr_instruct = seg_zero->seg[prog_counter];
            uint64_t word_0 = curr_instruct;
            word_0 = word_0 << (32);
            word_0 = word_0 >> (60);
            uint32_t opcode = (uint32_t) word_0;
            uint32_t A, B, C;
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
                if (opcode <= 3) {
                    if (opcode <= 1) {
                        if (opcode == 0) {
                            if (registers[C] != 0)
                                registers[A] = registers[B];
                        } else {
                            segment curr_seg = STACK[registers[B]];
                            registers[A] = curr_seg->seg[registers[C]];
                        } 
                    } else {
                        if (opcode == 2) {
                            segment curr_seg = STACK[registers[A]];
                            curr_seg->seg[registers[B]] = registers[C];
                        } else {
                            registers[A] = (registers[B] + registers[C]);
                        }
                    }
                } else {
                    if (opcode <= 5) {
                        if (opcode == 4) {
                            registers[A] = (registers[B] * registers[C]);
                        } else {
                            registers[A] = (registers[B] / registers[C]);
                        }
                    } else {
                        registers[A] = ~(registers[B] & registers[C]);
                    }
                }
            } else {
                if (opcode <= 10) {
                    if (opcode <= 8) {
                        if (opcode == 7) {
                            for (i = 0; i < segment_length; i++) {
                                segment temp = STACK[i];
                                if (temp != NULL)
                                    free(temp);
                            }
                            free(QUEUE);
                            free(STACK);
                            exit(EXIT_SUCCESS);
                        } else {
                            uint64_t word_2 = curr_instruct;
                            word_2 = word_2 << (58);
                            word_2 = word_2 >> (61);
                            B = (uint32_t) word_2;
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            segment m_seg = malloc(registers[C] * 
                                            sizeof(uint32_t) + sizeof(int));
                            for(i=0; i < (int)registers[C]; i++) {
                                m_seg->seg[i] = 0;
                            }
                            m_seg->length = registers[C];
                            if (queue_back - queue_front > 0) {
                                uint32_t location = 
                                            QUEUE[queue_front];
                                queue_front++;
                                STACK[location] = m_seg;
                                registers[B] = location;
                            } else {
                                if(segment_back+1 == 
                                    segment_length) {
                                    new_stack = malloc((segment_length*2) *
                                                         sizeof(new_stack));
                                    int j, len;
                                    len = segment_back;
                                    for(j=0; j < len; j++)
                                        new_stack[j] = STACK[j];
                                    free(STACK);
                                    STACK = new_stack;
                                    segment_length *=2;
                                    new_stack = NULL;
                                }
                                STACK[segment_back] = m_seg;
                                registers[B] = segment_back;
                                segment_back++;
                            }
                        }
                    }
                    else {
                        if (opcode == 9) {
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            segment unmap_seg = STACK[registers[C]];
                            free(unmap_seg);
                            STACK[registers[C]] = NULL;
                            if(queue_back+1 == queue_len) {
                                uint32_t *new_q = malloc(queue_len*2 * 
                                                            sizeof(uint32_t));
                                int k, length;
                                length = queue_back;
                                for(k=0; k < length; k++)
                                    new_q[k] = QUEUE[k];
                                free(QUEUE);
                                QUEUE = new_q;
                                queue_len *= 2;
                            }
                            QUEUE[queue_back] = registers[C];
                            queue_back++;
                        } else {
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            putchar(registers[C]);
                        }
                    }
                } else {
                    if (opcode <= 12) {
                        if (opcode == 11) {
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            char input = getchar();
                            if (input == EOF)
                                registers[C] = ~0;
                            else
                                registers[C] = (uint32_t)input;
                        } else { 
                            uint64_t word_2 = curr_instruct;
                            word_2 = word_2 << (58);
                            word_2 = word_2 >> (61);
                            B = (uint32_t) word_2;
                            uint64_t word_3 = curr_instruct;
                            word_3 = word_3 << (61);
                            word_3 = word_3 >> (61);
                            C = (uint32_t) word_3;
                            if (registers[B]!= 0) {
                                    segment curr_seg = STACK[registers[B]];
                                    segment new_seg = malloc(curr_seg->length 
                                            * sizeof(uint32_t) + sizeof(int));
                                    for(i=0; i< num_instructs; i++)
                                        new_seg->seg[i] = 0;
                                    new_seg->length = curr_seg->length;
                                    int lenn = new_seg->length;
                                    for (i = 0; i < lenn; i++)
                                        new_seg->seg[i] = curr_seg->seg[i];
                                    free(seg_zero);
                                    STACK[0] = NULL;
                                    STACK[0] = new_seg;
                                    seg_zero = new_seg;
                            }
                            prog_counter = (registers[C] - 1);
                        }
                    } else {
                            uint64_t word_1 = curr_instruct;
                            word_1 = word_1 << (36);
                            word_1 = word_1 >> (61);
                            A = (uint32_t) word_1;
                            uint64_t word_4 = curr_instruct;
                            word_4 = word_4 << (39);
                            word_4 = word_4 >> (39);
                            uint32_t value = (uint32_t) word_4;
                            registers[A] = value;
                    }
                }
            }
            prog_counter++;
        } 
    }
    return EXIT_SUCCESS;
}
