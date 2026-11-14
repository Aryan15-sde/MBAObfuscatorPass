#!/bin/bash
set -e

echo "=== 1. Building LLVM Pass ==="
cmake -B build -G Ninja >/dev/null
cmake --build build

echo ""
echo "=== 2. Generating IR & Running MBA Obfuscator ==="
clang -S -emit-llvm tests/test.c -o tests/test.ll
opt -load-pass-plugin ./build/MBAObfuscatorPass.so -passes="mba-obfuscator" -S tests/test.ll -o tests/test_obfuscated.ll

echo ""
echo "=== 3. Before vs After IR (Diff) ==="
diff tests/test.ll tests/test_obfuscated.ll || true

echo ""
echo "=== 4. Running Obfuscated Binary ==="
clang tests/test_obfuscated.ll -o tests/test_obfuscated
./tests/test_obfuscated
