// This header file will contain exports for the FFI functions implemented in the rust library

#pragma once

#include <stdint.h>

// Allocate an area of the specific size
// This is a trivial operation, but will demonstrate C->Rust->libc interaction
extern uint8_t* rustlib_alloc_test( size_t size);

// Alternate form of allocation that uses idomatic Rust method for getting a byte array on the heap
// NOTE: This implementation relies un an `unsafe{}`, and sometimes appears to fail without calling the oom handler.
extern uint8_t* rustlib_alloc_test2( size_t size);

// Another alternate that ues idomatic Rust to allocate space on the heap. Here, alloc AND init to 0 is done.
extern uint8_t* rustlib_alloc_test3( size_t size);

// Render the 3 given integers into a string using dynamic operaitons in Rust
extern char* rustlib_string_test( uint32_t a, uint32_t b, uint32_t c );