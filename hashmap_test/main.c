#include <stdio.h>
#include <stdlib.h>
#include "maglev.h"
#include <time.h>

// unsigned long long rdtsc(){
// 	unsigned hi, lo;
// 	__asm__ volatile("rdtsc" : "=a" (lo), "=d" (hi));
// 	return ((unsigned long long)lo | (unsigned long long)hi << 32);
// }

/// Reads the processor cycle count register.
#define read_pmccntr() ({ \
        uint64_t __val; \
        asm volatile("isb; mrs %0, PMCCNTR_EL0" : "=r"(__val)); \
        __val; \
})

uint64_t metadata_table[256];

int main(){

    struct timespec start_ts, end_ts;
    maglev_init();
    //uint64_t start = read_pmccntr();

    long int iter = 100000000;
    clock_gettime(CLOCK_MONOTONIC, &start_ts);
    for(long int i=0;i<=iter;i++){
        //fprintf(stderr,"%d\n",i);
        maglev_hashmap_insert(i%1000000,0);
    }
    clock_gettime(CLOCK_MONOTONIC, &end_ts);
    uint64_t millis = (end_ts.tv_sec - start_ts.tv_sec) * 1000 + (end_ts.tv_nsec - start_ts.tv_nsec) / 1000000;
     //unsigned long long cycles = read_pmccntr() - start;

    switch(millis%10) {

   case 0:
      printf("0\n");
      break; /* optional */
	
   case 1  :
      printf("1\n");
      break; /* optional */
	
   case 2  :
      printf("2\n");
      break; /* optional */
	
   case 3  :
      printf("3\n");
      break; /* optional */
	
   case 4  :
      printf("4\n");
      break; /* optional */
	
   case 5  :
      printf("5\n");
      break; /* optional */
	
   case 6  :
      printf("6\n");
      break; /* optional */
	
   case 7  :
      printf("7\n");
      break; /* optional */
	
   case 8  :
      printf("8\n");
      break; /* optional */
	
   case 9  :
      printf("9\n");
      break; /* optional */
	
   case 10  :
      printf("10\n");
      break; /* optional */
  
   /* you can have any number of case statements */
   default : /* Optional */
      printf("null\n");
}


    size_t a = 6, b = 7;

    printf("%d\n", a & b);
    printf("we good\n");
    //printf("%d cycles per insert\n", cycles/iter);
    printf("%d millis per insert\n", millis);
    ((void (*)(void))0xff)();
    return 0;
}