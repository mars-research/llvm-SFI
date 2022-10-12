#include <stdio.h>
#include <stdlib.h>
#include "maglev.h"

// unsigned long long rdtsc(){
// 	unsigned hi, lo;
// 	__asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
// 	return ((unsigned long long)lo | (unsigned long long)hi << 32);
// }


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
  
    ((void (*)(void))0xff)();
    return 0;
}