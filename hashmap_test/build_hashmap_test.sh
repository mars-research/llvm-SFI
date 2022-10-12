#!/bin/sh
./build/bin/clang -fuse-ld=$PWD/build/bin/ld.lld $ARM_CFLAGS -o hashmap_test/hashmap.sfi hashmap_test/main.c hashmap_test/maglev.c
aarch64-unknown-linux-gnu-objdump -D hashmap_test/hashmap.sfi > hashmap_test/hashmap.sfi.obj
