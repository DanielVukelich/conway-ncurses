#include "bit_accessor.h"
#include "errcode.h"

#include <stdlib.h>
#include <string.h>

const unsigned int WORD_SIZE = sizeof(uint32_t);
const unsigned int WORD_BITS = WORD_SIZE * 8;
const unsigned int SUBWORD_MASK_LEN = 6;  //log_2(word_size - 1)
const unsigned int SUBWORD_MASK = 0b11111; //SUBWORD_MASK_LEN '1' bits

unsigned int get_word_index(unsigned int bit_index){
    return bit_index >> (SUBWORD_MASK_LEN - 1);
}

uint32_t get_bit_mask(unsigned int bit_index){
    unsigned int subword_index = bit_index & SUBWORD_MASK;
    if(subword_index >= WORD_BITS)
        return 0;
    return ((unsigned int)1 << subword_index);
}

unsigned int read_bit(unsigned int bit_index, uint32_t* bitmap){
    unsigned int word_index = get_word_index(bit_index);
    uint32_t bit_mask = get_bit_mask(bit_index);
    return bitmap[word_index] & bit_mask;
}

void set_bit_on(unsigned int bit_index, uint32_t* bitmap){
    unsigned int word_index = get_word_index(bit_index);
    uint32_t bit_mask = get_bit_mask(bit_index);

    bitmap[word_index] |= bit_mask;
}

void set_bit_off(unsigned int bit_index, uint32_t* bitmap){
    unsigned int word_index = get_word_index(bit_index);
    uint32_t bit_mask = get_bit_mask(bit_index);

    bitmap[word_index] &= ~bit_mask;
}

void flip_bit(unsigned int bit_index, unsigned int* bitmap){
    unsigned int word_index = get_word_index(bit_index);
    uint32_t bit_mask = get_bit_mask(bit_index);

    bitmap[word_index] ^= bit_mask;
}

void set_bit(bit_accessor* accessor, unsigned int bit_index, bool value){
    if(bit_index >= accessor->num_bits)
        return;
    if(value)
        set_bit_on(bit_index, accessor->bitmap);
    else
        set_bit_off(bit_index, accessor->bitmap);
}

void toggle_bit(bit_accessor* accessor, unsigned int bit_index){
    if(bit_index >= accessor->num_bits)
        return;
    flip_bit(bit_index, accessor->bitmap);
}

bool get_bit(bit_accessor* accessor, unsigned int bit_index){
    if(bit_index >= accessor->num_bits)
        return false;
    return read_bit(bit_index, accessor->bitmap);
}

unsigned int num_words_for_bitmap(unsigned int num_bits){
    unsigned int min_words_needed = (num_bits / WORD_BITS);

    if((num_bits) % WORD_BITS == 0)
        return min_words_needed;
    return (min_words_needed + 1);
}

unsigned int random_word(int seed_rate){
    //TODO: Generate this more efficiently
    unsigned int word = 0;
    for(unsigned int i = 0; i < WORD_BITS; ++i){
        if(rand() % seed_rate == 0)
            word = (word << 1) + 1;
        else
            word = (word << 1);
    }
    return word;
}

void clear_all_bits(bit_accessor* accessor){
    memset(accessor->bitmap, 0, accessor->num_words * WORD_SIZE);
}

int init_accessor(bit_accessor* accessor, unsigned int num_bits){
    accessor->num_words = num_words_for_bitmap(num_bits);
    unsigned int bitmap_len = WORD_SIZE * accessor->num_words;

    accessor->bitmap = malloc(bitmap_len);
    if(accessor->bitmap == NULL)
        return OUT_OF_MEM;

    accessor->num_bits = num_bits;
    clear_all_bits(accessor);
    return NO_ERR;
}

void free_accessor(bit_accessor* accessor){
    free(accessor->bitmap);
}
