#include "../include/linux/bitmap.h"
#include "../include/string.h"

// create bitmap
void bitmap_make(bitmap_t *map, char *bits, u32 length, u32 offset)
{
    map->bits = bits;
    map->length = length;
    map->offset = offset;
}

// init
void bitmap_init(bitmap_t *map, char *bits, u32 length, u32 start)
{
    memset(bits, 0, length);
    bitmap_make(map, bits, length, start);
}


bool bitmap_test(bitmap_t *map, idx_t index)
{
    // get index
    idx_t idx = index - map->offset;


    u32 bytes = idx / 8;


    u8 bits = idx % 8;


    return (map->bits[bytes] & (1 << bits));
}

// set val
void bitmap_set(bitmap_t *map, idx_t index, bool value)
{

    idx_t idx = index - map->offset;


    u32 bytes = idx / 8;


    u8 bits = idx % 8;
    if (value)
    {
        // set 1
        map->bits[bytes] |= (1 << bits);
    }
    else
    {
        // set 0
        map->bits[bytes] &= ~(1 << bits);
    }
}


int bitmap_scan(bitmap_t *map, u32 count)
{
    int start = EOF;
    u32 bits_left = map->length * 8;
    u32 next_bit = 0;
    u32 counter = 0;


    while (bits_left-- > 0)
    {
        if (!bitmap_test(map, map->offset + next_bit))
        {

            counter++;
        }
        else
        {

            counter = 0;
        }


        next_bit++;


        if (counter == count)
        {
            start = next_bit - count;
            break;
        }
    }


    if (start == EOF)
        return EOF;


    bits_left = count;
    next_bit = start;
    while (bits_left--)
    {
        bitmap_set(map, map->offset + next_bit, true);
        next_bit++;
    }

    //return index
    return start + map->offset;
}