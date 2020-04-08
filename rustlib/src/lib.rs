#![crate_type = "staticlib"]

#![no_std]
#![feature(lang_items)]

extern crate panic_halt;

#[macro_use]
extern crate alloc;
extern crate cortex_m;

// Implement an allocator with libc malloc and free
// Import malloc and free from libc:
#[link(name = "c")]
extern "C" {
    #[no_mangle]
    pub fn malloc( new_size: usize ) -> *mut u8;
    #[no_mangle]
    pub fn free( ptr: *mut u8 );
}

// Define allocator that just calls down to libc malloc/free
struct LibCAllocator;

unsafe impl alloc::alloc::GlobalAlloc for LibCAllocator {
    unsafe fn alloc(&self, _layout: alloc::alloc::Layout) -> *mut u8 {
        malloc( _layout.size() )
    }
    unsafe fn dealloc(&self, _ptr: *mut u8, _layout: alloc::alloc::Layout)
    {
        free( _ptr );
    }
}

// Assign our wrapper as the global allocator
#[global_allocator]
static A: LibCAllocator = LibCAllocator;


// alloc will use the configured global allocator
use alloc::string::String;
use alloc::vec::Vec;

// cstr_core is needed to create C-like strings
extern crate cstr_core;
use cstr_core::CString;
use cstr_core::c_char;

use alloc::boxed::Box;
use alloc::alloc::Layout;

#[no_mangle]
pub fn rustlib_alloc_test( size : usize ) -> *const u8 {
    let lay = Layout::from_size_align( size, 4 ).unwrap();
    let alloced = unsafe{ alloc::alloc::alloc( lay ) };
    alloced as *const u8
}

#[no_mangle]
pub fn rustlib_alloc_test2( size : usize ) -> *const u8 {
    let mut v : Vec<u8> = Vec::with_capacity(size );
    unsafe { v.set_len( size ) };
    let bs = v.into_boxed_slice();
    let b = Box::leak( bs ).as_ptr();
    b as *const u8
}

#[no_mangle]
pub fn rustlib_alloc_test3( size : usize ) -> *const u8 {
    let v : Vec<u8> = vec![0; size];
    let bs = v.into_boxed_slice();
    let b = Box::leak( bs ).as_ptr();
    b as *const u8
}

#[no_mangle]
pub fn rustlib_string_test( a : u32, b : u32, c : u32 ) -> *const c_char
{
    // These string operatings are done intentionally to:
    //  1) give us breakpoints to see how the Rust code behaves
    //  2) possibly generate several heap operations (malloc/free)
    let mut output = String::from("");

    // Note: here, the format! macro is coming from the alloc crate
    output.push_str(&format!("{} and ",a) );
    output.push_str(&format!("{} and ",b) );
    output.push_str(&format!("{}",c) );

    // Current limitation: to make a CString from a Rust String, this likely
    // cause a copy operation
    let s = CString::new( output ).unwrap();
    let p = Box::into_raw(s.into_bytes_with_nul().into_boxed_slice());
    p as *const c_char
}

// required: define how Out Of Memory (OOM) conditions should be handled
// *if* no other crate has already defined `oom`
#[lang = "oom"]
#[no_mangle]
pub fn rust_oom( _layout : alloc::alloc::Layout ) -> ! {
    // Set a breakpoint to catch the debugger
    cortex_m::asm::bkpt();

    loop {};
}