#!/bin/sh
rm MI.sfi
rm MI.nosfi
./build/bin/clang -g -O3 -fomit-frame-pointer -fuse-ld=$PWD/build/bin/ld.lld  -o hashmap_test/hashmap.sfi hashmap_test/main.c hashmap_test/maglev.c
clang -g -O3 -fuse-ld=$PWD/build/bin/ld.lld  -o hashmap_test/hashmap.nosfi hashmap_test/main.c hashmap_test/maglev.c
objdump -DS hashmap_test/hashmap.sfi > hashmap_test/hashmap.sfi.obj
objdump -DS hashmap_test/hashmap.nosfi > hashmap_test/hashmap.nosfi.obj
