#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "rust_statiblic_libc_allocator.h"

int main()
{
    volatile uint32_t a,b,c;

    // First, we'll test malloc from C; we should be able to see this string in the heap
    char* c_heap_string = strdup("string test");
    (void)c_heap_string; // suppress "unused" warning

    // Try allocating RAM with C -> Rust -> C malloc (trivial wrapper)
    uint8_t* rust_heap_alloc = rustlib_alloc_test( 16 );
    memset( rust_heap_alloc, 0xFF, 16 );

    // We will use the libc prng to make changing values to turn into strings
    srand(0);

    while( 1 )
    {
        // Generate values from PRNG
        a = rand() % 1000;
        b = rand() % 1000;
        c = rand() % 1000;

         // Call function that should internally do string operations, which rely on malloc
        char* rust_heap_string = rustlib_string_test( a, b, c );

        // At this point, there should be several string operations by rustlib visible on the heap
        volatile size_t rust_heap_string_len = strlen( rust_heap_string );

        // TODO: printing via semihosting
        // printf("string: \"%s\", len = %d\n", rust_heap_string, strlen(rust_heap_string) );
        (void)rust_heap_string_len; // suppress "unused" warning
        // In lieu of printf, catch debugger to inspect results and state of the heap
        // asm("BKPT");

        free( rust_heap_string );


        // We expect the above example to generate a string of almost constant length.
        // Here, we do a torture test for any one of the rustlib_alloc_testX functions.
        volatile size_t rand_alloc_size;
        rand_alloc_size = rand() % (10*1024);

        uint8_t* (*alloc_func)(size_t);
        // Select one of the alloc functions to torture test:
        // alloc_func = rustlib_alloc_test;    // Simple wrapper, relies on unsafe {}, known working
        // alloc_func = rustlib_alloc_test2;   // Idomatic approach, relies on unsafe{}, sometimes fails to alloc
        alloc_func = rustlib_alloc_test3;      // Idiomatic approach with 0 init, no unsafe, known working

        uint8_t* rust_heap_alloc_rand = alloc_func( rand_alloc_size );

        // Check that we got a valid RAM pointer: non-null, within physical SRAM, and 32bit aligned.
        // TODO: import exact heap bounds from linker/newlib/sbrk for a stronger check.
        // NOTE: these are specific to the STM32L082 being tested (20K SRAM)
        if( ( NULL == rust_heap_alloc_rand) ||
            ( 0x20000000u > (uint32_t)rust_heap_alloc_rand ) || ( 0x20005000u < (uint32_t)rust_heap_alloc_rand ) ||
            ( (uint32_t)rust_heap_alloc_rand % 4) )
        {
            asm("BKPT");        // Bad allocation
        }

        // Fill allocated memory to examine the high water mark, if we are leaking, etc.
        memset( (char*)rust_heap_alloc_rand, 0xFF, rand_alloc_size );

        free( rust_heap_alloc_rand );
    }

    return 0;
}
