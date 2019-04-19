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
#include <string.h>
#include "mem.h"
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

typedef struct A {
    int length;
    int size;
    char *array;
} * A;

typedef struct T {
	struct A array;
	int length;
	int head;
} * T;

typedef struct UM
{
    uint32_t prog_counter;
    uint32_t *registers;
    T seg_mem;
    T unmapped_segs;
} * UM;

typedef struct segment
{
    int length;
    uint32_t seg[];
} * segment;

static inline void Array_resize(A array, int length) {
	if (length == 0)
		FREE(array->array);
	else if (array->length == 0)
		array->array = ALLOC(length*array->size);
	else
		RESIZE(array->array, length*array->size);
	array->length = length;
}

static inline void expand(T seq) {
	int n = seq->array.length;
	Array_resize(&seq->array, 2*n);
	if (seq->head > 0)
		{
			void **old = &((void **)seq->array.array)[seq->head];
			memcpy(old+n, old, (n - seq->head)*sizeof (void *));
			seq->head += n;
		}
}

static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
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

static inline void ArrayRep_init(A array, int length, int size,
	void *ary) {
	array->length = length;
	array->size   = size;
	if (length > 0)
		array->array = ary;
	else
		array->array = NULL;
}

static inline void Array_free(A *array) {
	FREE((*array)->array);
	FREE(*array);
}

static inline T Seq_new(int hint) {
	T seq;
	NEW0(seq);
	if (hint == 0)
		hint = 16;
	ArrayRep_init(&seq->array, hint, sizeof (void *),
		ALLOC(hint*sizeof (void *)));
	return seq;
}

static inline void Seq_free(T *seq) {
	Array_free((A *)seq);
}
static inline int Seq_length(T seq) {
	return seq->length;
}
static inline void *Seq_get(T seq, int i) {
	return ((void **)seq->array.array)[
	       	(seq->head + i)%seq->array.length];
}
static inline void *Seq_put(T seq, int i, void *x) {
	void *prev;
	prev = ((void **)seq->array.array)[
	       	(seq->head + i)%seq->array.length];
	((void **)seq->array.array)[
		(seq->head + i)%seq->array.length] = x;
	return prev;
}

static inline void *Seq_remlo(T seq) {
	int i = 0;
	void *x;
	x = ((void **)seq->array.array)[
	    	(seq->head + i)%seq->array.length];
	seq->head = (seq->head + 1)%seq->array.length;
	--seq->length;
	return x;
}
static inline void *Seq_addhi(T seq, void *x) {
	int i;
	if (seq->length == seq->array.length)
		expand(seq);
	i = seq->length++;
	return ((void **)seq->array.array)[
	       	(seq->head + i)%seq->array.length] = x;
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
        UM UM = malloc(sizeof(struct UM));
        UM->prog_counter = 0;
        UM->registers = malloc(NUM_REG * sizeof(uint32_t));
        int i;
        for (i = 0; i < NUM_REG; i++)
            UM->registers[i] = 0;

        UM->seg_mem = Seq_new(SEQ_HINT);

        segment seg_zero = malloc(num_instructs * sizeof(uint32_t)+sizeof(int));
        for(i=0; i< num_instructs; i++) {
            seg_zero->seg[i] = 0;
        }
        seg_zero->length = num_instructs;
        Seq_addhi(UM->seg_mem, seg_zero);

        UM->unmapped_segs = Seq_new(SEQ_HINT);
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
        while (1) 
        {
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
                            segment curr_seg = Seq_get(UM->seg_mem, *regB);
                            *regA = curr_seg->seg[*regC];
                        } 
                    } else {
                        if (opcode == 2) {
                            segment curr_seg = Seq_get(UM->seg_mem, *regA);
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
                            for (i = 0; i < Seq_length(UM->seg_mem); i++) {
                                segment temp = Seq_get(UM->seg_mem, i);
                                if (temp != NULL)
                                    free(temp);
                            }
                            Seq_free(&(UM->unmapped_segs));
                            Seq_free(&(UM->seg_mem));
                            free(UM);
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
                            segment segment = malloc(*regC * sizeof(uint32_t) + sizeof(int));
                            for(i=0; i < (int)*regC; i++) {
                                segment->seg[i] = 0;
                            }
                            segment->length = *regC;
                            if (Seq_length(UM->unmapped_segs) > 0) {
                                uint32_t location = (uint32_t)(uintptr_t)
                                Seq_remlo(UM->unmapped_segs);
                                Seq_put(UM->seg_mem, location, segment);
                                *regB = location;
                            } else {
                                Seq_addhi(UM->seg_mem, segment);
                                *regB = (Seq_length(UM->seg_mem) - 1);
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
                            segment unmap_seg = Seq_get(UM->seg_mem, *regC);
                            free(unmap_seg);
                            Seq_put(UM->seg_mem, *regC, NULL);
                            Seq_addhi(UM->unmapped_segs, (void *)(uintptr_t)*regC);
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
                                    segment curr_seg = Seq_get(UM->seg_mem, *regB);
                                    segment new_seg = malloc(curr_seg->length * sizeof(uint32_t) + sizeof(int));
                                    for(i=0; i< num_instructs; i++) {
                                        new_seg->seg[i] = 0;
                                    }
                                    new_seg->length = curr_seg->length;
                                    for (i = 0; i < new_seg->length; i++) {
                                        new_seg->seg[i] = curr_seg->seg[i];
                                    }
                                    free(seg_zero);
                                    Seq_put(UM->seg_mem, SEG_ZERO, NULL);
                                    Seq_put(UM->seg_mem, SEG_ZERO, new_seg);
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
