#!/bin/sh
./build/bin/clang -O3 -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmapsfi hashmap_test/main.c hashmap_test/maglev.c
clang -O3 -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmapnosfi hashmap_test/main.c hashmap_test/maglev.c
aarch64-unknown-linux-gnu-objdump -D hashmap_test/hashmapsfi > hashmap_test/hashmap.sfi.obj
