#ifndef _MAGLEV_DPDK_H_
#define _MAGLEV_DPDK_H_

#define CAPACITY ((1ULL << 20) * 16)

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

/* maglev KV pair */
struct maglev_kv_pair {
    unsigned long long key;
    unsigned long long value;
};


struct maglev_hashmap {
    struct maglev_kv_pair *pairs;
};

// Maglev Init
void maglev_init(void);

// Maglev process frame
void maglev_hashmap_insert(unsigned long long key, unsigned long long value);

#endif /* _MAGLEV_DPDK_H_ */
