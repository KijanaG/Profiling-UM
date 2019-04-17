/*
 * Martina Tan (mtan02) & Kunaal Bhargava (kbharg01) 
 * Comp 40 Spring 2019
 * Arith
 *
 * bitpack implementation
 * 3/10/19
 *                                              
 */
#include <stdbool.h>
#include <stdint.h>
#include "except.h"
#include <stdio.h>
#include "assert.h"
#include "except.h"
#include "bitpack.h"

const unsigned MAX_NUM_BITS = 64;

Except_T Bitpack_Overflow = { "Overflow packing bits" };

bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= MAX_NUM_BITS);
        unsigned bit_shift;
        uint64_t curr;

        bit_shift = MAX_NUM_BITS - width;
        
        curr = n;
        curr = curr << bit_shift;
        curr = curr >> bit_shift;

        if (curr == n && bit_shift != MAX_NUM_BITS) {
                return true;
        } else {
                return false;
        }
}

bool Bitpack_fitss(int64_t n, unsigned width)
{
        assert(width <= MAX_NUM_BITS);
        unsigned bit_shift;
        int64_t curr;

        bit_shift = MAX_NUM_BITS - width;

        curr = n;
        curr = curr << bit_shift;
        curr = curr >> bit_shift;

        if (curr == n && bit_shift != MAX_NUM_BITS) {
                return true;
        } else {
                return false;
        }
}

uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= MAX_NUM_BITS);
        assert(width + lsb <= MAX_NUM_BITS);

        if (width == 0) {
                return 0;
        }

        word = word << (MAX_NUM_BITS - (width + lsb));
        word = word >> (MAX_NUM_BITS - width);

        return word;
}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= MAX_NUM_BITS);
        assert(width + lsb <= MAX_NUM_BITS);

        if (width == 0) {
                return 0;
        }

        int64_t signed_word = word;
        signed_word =  signed_word << (MAX_NUM_BITS - (width + lsb));
        signed_word = signed_word >> (MAX_NUM_BITS - (width));

        return signed_word;
}

uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        assert(width <= MAX_NUM_BITS);
        assert(width + lsb <= MAX_NUM_BITS);

        if (Bitpack_fitsu(value, width) != true) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t bit_mask = ~0;

        bit_mask = bit_mask << (MAX_NUM_BITS -  (width +lsb));
        bit_mask = bit_mask >> (MAX_NUM_BITS - width);
        bit_mask = bit_mask << lsb;
        bit_mask = ~bit_mask;

        value = value << lsb;

        word = word & bit_mask;
        word = word | value;

        return word;
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
                      int64_t value)
{
        assert(width <= MAX_NUM_BITS);
        assert(width + lsb <= MAX_NUM_BITS);

        if (Bitpack_fitss(value, width) != true) {
                RAISE(Bitpack_Overflow);
        }

        uint64_t bit_mask = ~0;

        bit_mask = bit_mask << (MAX_NUM_BITS - (width +lsb));
        bit_mask = bit_mask >> (MAX_NUM_BITS - width);
        bit_mask = bit_mask << lsb;
        bit_mask = ~bit_mask;

        value = value << lsb;

        word = word & bit_mask;
        word = word | value;

        return word;
}

