#include <stdio.h>
#include <stdlib.h>
#include "maglev.h"

// unsigned long long rdtsc(){
// 	unsigned hi, lo;
// 	__asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
// 	return ((unsigned long long)lo | (unsigned long long)hi << 32);
// }

#define read_pmccntr() ({ \
        uint64_t __val; \
        asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(__val)); \
        __val; \
})

int main(){

    maglev_init();
    //uint64_t start = read_pmccntr();

    long int iter = 1000000;
    for(long int i=0;i<=iter;i++){
        maglev_hashmap_insert(i%1000000,0);
    }
    
     //unsigned long long cycles = read_pmccntr() - start;



    size_t a = 6, b = 7;

    printf("%d\n", a & b);
    //printf("we good\n");
     printf("%d\n",cycles/iter);    
    ((void (*)(void))0xff)();
    return 0;
}