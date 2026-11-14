# MBAObfuscatorPass

An out-of-tree **LLVM New Pass Manager** transformation pass that performs **Mixed Boolean-Arithmetic (MBA) obfuscation** on LLVM IR.

## How It Works

Each integer `add` instruction (`a + b`) is rewritten using the bitwise identity `(a ^ b) + ((a & b) << 1)`, preserving the exact runtime output while hiding straightforward arithmetic from static decompilers.

## Build & Run

    cmake -B build -G Ninja && cmake --build build
    clang -S -emit-llvm tests/test.c -o tests/test.ll
    opt -load-pass-plugin ./build/MBAObfuscatorPass.so -passes="mba-obfuscator" -S tests/test.ll -o tests/test_obfuscated.ll

## Verify Output

    diff tests/test.ll tests/test_obfuscated.ll
    clang tests/test_obfuscated.ll -o tests/test_obfuscated && ./tests/test_obfuscated
