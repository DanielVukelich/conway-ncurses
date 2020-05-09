#ifndef _BIT_ACCESSOR_H
#define _BIT_ACCESSOR_H

#include <stdint.h>
#include <stdbool.h>

typedef struct bit_accessor_t{
    uint32_t* bitmap;
    unsigned int num_bits;
    unsigned int num_words;
}bit_accessor;

int init_accessor(bit_accessor* accessor, unsigned int num_bits);
void free_accessor(bit_accessor* accessor);

void clear_all_bits(bit_accessor* accessor);
void set_bit(bit_accessor* accessor, unsigned int bit_index, bool value);
bool get_bit(bit_accessor* accessor, unsigned int bit_index);
void toggle_bit(bit_accessor* accessor, unsigned int bit_index);

#endif
