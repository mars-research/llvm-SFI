#!/bin/sh
./build/bin/clang -march=armv8.2-a+pauth  -g -O3 -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmap.sfi hashmap_test/main.c hashmap_test/maglev.c
clang -g -O3 $ARM_CFLAGS -o hashmap_test/hashmap.nosfi hashmap_test/main.c hashmap_test/maglev.c
objdump -DS hashmap_test/hashmap.sfi > hashmap_test/hashmap.sfi.obj
objdump -DS hashmap_test/hashmap.nosfi > hashmap_test/hashmap.nosfi.obj
