#!/bin/sh
# ./build/bin/clang -g -O1 -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmap.sfi hashmap_test/main.c hashmap_test/maglev.c
# clang -g -O3 -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmap.nosfi hashmap_test/main.c hashmap_test/maglev.c
# aarch64-unknown-linux-gnu-objdump -DS hashmap_test/hashmap.sfi > hashmap_test/hashmap.sfi.obj
# aarch64-unknown-linux-gnu-objdump -DS hashmap_test/hashmap.nosfi > hashmap_test/hashmap.nosfi.obj
rm -rf ../MI.sfi
rm -rf ../MI.nosfi
./build/bin/clang -g -O3 -fuse-ld=$PWD/build/bin/ld.lld -c -o hashmap_test/maglev.o hashmap_test/maglev.c
clang -g -O3  -o hashmap_test/hashmap.sfi hashmap_test/main.c hashmap_test/maglev.o
clang -g -O3  -o hashmap_test/hashmap.nosfi hashmap_test/main.c hashmap_test/maglev.c
objdump -DS hashmap_test/hashmap.sfi > hashmap_test/hashmap.sfi.obj
objdump -DS hashmap_test/hashmap.nosfi > hashmap_test/hashmap.nosfi.obj