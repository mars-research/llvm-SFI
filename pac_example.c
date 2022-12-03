#include <stdio.h>

// this is a 256 byte buffer and it's 256 bytes aligned
// we allocate doubled of it so we can store a metadata before it;
char buff[1<<10]__attribute__ ((aligned (1<<9))); 
char *buff_ptr = buff+(1<<9);

#include <stdint.h>

int main() {
        uint64_t context = 0x233;

        void *ptr = buff_ptr;
        void * signed_ptr = ptr;
        *((unsigned long *)buff_ptr - 1) = context;
        printf("      unsigned pointer: %p\n", ptr);

        asm("pacda %0, %2" : "=r"(signed_ptr) : "0"(signed_ptr), "r"(context));
        printf("        signed pointer: %p\n", signed_ptr);

        unsigned long sz_bits = 0x9;
        sz_bits = sz_bits << 56;
        unsigned long * signed_ptr_with_sz = (unsigned long long) ptr + sz_bits;
        printf("((1<<9)<<56) 0x%lx\n",sz_bits);
        printf(" signed_ptr_with_sz pointer: %p\n", signed_ptr_with_sz);
        asm("pacda %0, %2" : "=r"(signed_ptr_with_sz) : "0"(signed_ptr_with_sz), "r"(context));
        printf(" signed_ptr_with_sz pointer before authenticated: %p\n", signed_ptr_with_sz);
        unsigned long sz = 1 << ((unsigned long)signed_ptr_with_sz>>56);
        unsigned long metadata = *(unsigned long *)((((unsigned long)signed_ptr_with_sz & 0xFFFFFFFFFFFF)>>sz<<sz) - 8);
        printf("metadata %lx \n",metadata);
        asm("autda %0, %2" : "=r"(signed_ptr_with_sz) : "0"(signed_ptr_with_sz), "r"(metadata));
        printf("signed_ptr_with_sz pointer after authenticated %p\n", signed_ptr_with_sz);
        printf("deallocating signed_ptr_with_sz %lx\n",*signed_ptr_with_sz);

        void *authenticated = signed_ptr;
        asm("autda %0, %2" : "=r"(authenticated) : "0"(authenticated), "r"(context));
        printf(" authenticated pointer: %p\n", authenticated);

        //what if we use the wrong context to authenticate?
        void *noauthenticated = signed_ptr;
        asm("autda %0, %2" : "=r"(noauthenticated) : "0"(noauthenticated), "r"(0x666));
        printf("aut with wrong context: %p\n", noauthenticated);

        // what if we don't authenticate?
        int * signed_int_ptr = signed_ptr;
        printf("aut with wrong context: %d\n", *signed_int_ptr);
}

// int main(){
//     char * pac_ptr = (unsigned long)buff_ptr 
//     asm("MOV X2, 0x9");
//     asm("lsl x2, x2, #56");
//     asm("orr x1, x1, x2");
//     asm("PACIA X1, X2");
//     asm("AUTIA X1, X2");
//     printf("buff %p size 0x%x \nbuff_ptr %p\n",buff,1<<10,buff_ptr);
//     // printf("hello world\n");
// }