Embedded Rust staticlib with libc allocation
=========

This repository contains a technology test/template for enabling the use of dynamic allocation in Rust in the context of
a Rust staticlib being called from C code written for an embedded microcontroller target, e.g. ARM Cortex M0, M3, M4, M7.

The motivation for this test was to discover a way to bring the features, productivity, and ecosystem of Rust to existing
embedded C code bases. While constrained to the `no_std` environment, addition of dynamic alloc to Rust enables many common
patterns.

### How it Works

Inside the Rust library, a GlobalAllocator is implemented which calls back to the libc `malloc` and `free` functions.\
This assumes that a libc runtime has already been set up by the startup sequence in C or assembly. \
So, when using this Rust static library, implemented functions can be called via C-to-Rust FFI, which will then use Rust-to-C FFI to allocate and free heap memory as needed.

This should allow the Rust code to use the existing heap area as defined by the C program which is linked into, and
Rust and C allocations should be able to coexist on the same heap.

### Demonstration

This demonstration provide a template for use of source-level Rust libraries as part of a C codebase for an embedded target. It also provides a proof of concept of a solution to enabling useful dynamic memory operations from Rust, shown her by some string manipulation functions in Rust which invariably are using heap allocation.

In the demo, three (pseudo)random positive integer values are generated and then rendered into a string part-by-part by the Rust library. \
In `rustlib_string_test`, you will see that manipulating a Rust String into a C-compatible string (i.e. null terminated) string on the heap, such that it is not desstructed as the Rust function exists, and a raw pointer is returned to the caller does take some maneuvering.

For the sake of keeping things lean, this example does not go as far as setting up a UART for printf or printing via debugger semihosting, so the success of the operations will have to be observed by viewing the heap memory directly with your debugger.

The demonstration also includes a "torture test"  in the main loop for larger allocations, and several approaches to achieveing allocation from within the Rust code.

### Code

The repo consists of a straightforward C program in the root. The Rust library is implemented in the `rustlib/` directory.

A single Makefile will build all the C and Rust code.

### Makefile

The included Makefile is almost identical to the automatically generated Makefile from the [STM32CubeMX tool](https://www.st.com/en/development-tools/stm32cubemx.html) from ST Microelectronics.

Small extensions are made to include the building of the Rust library code. \
There are some subtle changes that are made to the linking order of the objects to prevent symbol collsions, as discussed below in "Unintended Language Hybridization".

The CSP and HAL files generated in the bundle (specific to STM32L0) are included in this repo, but many are not used. \
**Note**: The STM32 HAL is never set up in this example. \
**Note**: The MCU clock IS set up in this example, as generated by the STM tool, into the `SystemInit` function. The MCU is set up for a 32Mhz core clock from the HSI16 + PLL. This does not require any external component support.

### Building

Just run `make`, which will build the C code and build the library in `rustlib/` with `cargo` and link.

The contents of `rustlib/` are just a plain cargo project, and can be built separeatly with `cd rustlib; cargo build`.

**The nightly rustc toolchain is required**, `rustc` and `cargo` are expected to be provided by your shell's path.

### Hardware

The project in this repositry is set up specifically for an STM32L082CZ microcontroller, but it should work for any STM32L0 part, given that flash and RAM resources are not overrun. The code has been tested on said MCU, in a [Murata CMWX1ZZABZ-078](https://wireless.murata.com/type-abz-078.html) module, as provided on the [STMicro B-L072Z-LRWAN1](https://www.st.com/en/evaluation-tools/b-l072z-lrwan1.html) eval board.

The ELF generated by the Makefile is directly loadable onto the micro with your tools of choice.

A favorite approach of the author's is to:
- [Reflash the STLink](https://www.segger.com/products/debug-probes/j-link/models/other-j-links/st-link-on-board/) on the dev board into a minimal Segger JLink compatible debugger (non-commercial only)
- Use the [Segger Ozone](https://www.segger.com/products/development-tools/ozone-j-link-debugger/) debugging tool to load the elf and run the code.

Other various mixtures of the standard tools should work, too: \
`arm-none-eabi-gdb`, `openocd`, `JLinkGDBServer`, etc

### Known Limitations

#### Idiomatic allocation
In `lib.rs`, you will see thee distinct implementations of a function testing simple allocation from Rust.
No one way is apparently the best, between using unsafe, failling allocation, or requiring explicit init of the underlying memory.

This is not expected to be a needed functionality, allocating from C via Rust and providing the pointer back to C, but it is expected to come up in the case of modifying existing heap-allocated structures and memory passed into Rust as an argument.

#### Makefile setup
This template is only set up to build one Rust library which is specifically enumerated in the Makefile. This would need to be adapted to whatever build system you have wrapped around whichever SDK you are using for your embedded code project.

The [rust-target-cmake](https://github.com/berkowski/rust-target-cmake) repository contains a great example of how delegation of building the rust library in a CMake environment can be handled.

#### Unintended Language Hybridization (!)

If you look at the ELF file closely, or examing on your hardware target with a debugger, you will see that the `memset` function called from the C code actually gets linked to the implementaion in `mem.rs`, provided by the `compiler-builtins` crate, and not the libc implementations!

This is a result of the linking order set up in the Makefile. Without that linking order, there will be an error for multiple definitions of `memset` and other stdlib functions provided by libc/newlib in your compiler distribution.

Even more surprisingly: If you do `float` operations in C on a soft FPU micro, even the `__aebi_fmul` operations will end up being linked to the Rust implementations!

This issue has been raised here: https://github.com/rust-lang/compiler-builtins/issues/345 \
and some manual workarounds do exist, e.g. limiting symbol export from the rustlib.

### Next steps.

The setup of the GlobalAllocator in the Rust library could likely be moved to it's own crate and provided as a general construct for this same use case. (This may already be published by someone else)

An example leveraging a significantly more complex library "out of the box" will be built on this template, e.g. serde for serialization and deserialization of C structs. (TODO, add link here)

## License

Public domain

### Contribution

Public domain

As this is a technology demonstration and a skill builder for the author, instructive issues and PR's are welcome.