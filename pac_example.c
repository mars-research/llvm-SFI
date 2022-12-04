#include <stdio.h>

// this is a 256 byte buffer and it's 256 bytes aligned
// we allocate doubled of it so we can store a metadata before it;
char buff[1<<10]__attribute__ ((aligned (1<<9))); 
char *buff_ptr = buff+(1<<9);

#include <stdint.h>
void * pac(void * ptr, uint64_t size){
        uint64_t context = 0x233;

        void * signed_ptr = ptr;
        *((uint64_t *)ptr - 1) = context;

        uint64_t sz_bits = size;
        sz_bits = sz_bits << 56;

        //uint64_t pac_context = size ^ context;
        signed_ptr = (uint64_t *)((uint64_t) signed_ptr + sz_bits);
        printf("signing ptr %p with %lx\n",signed_ptr, context);
        asm("pacda %0, %2" : "=r"(signed_ptr) : "0"(signed_ptr), "r"(context));
        printf("paced_ptr %p\n",signed_ptr);
        return signed_ptr;
}

void * auth(void * ptr){
        uint64_t * actual_ptr =  (uint64_t *) ((uint64_t) ptr & 0xFFFFFFFFFFFF);
        uint64_t sz = ((uint64_t)ptr>>56);
        uint64_t * orginal_ptr = (uint64_t *)((uint64_t)ptr >>sz<<sz);
        uint64_t * orginal_ptr_wotag = (uint64_t *) ((uint64_t)orginal_ptr & 0xFFFFFFFFFFFF);
        //printf("sz %lx orginal_ptr %p\n",sz, orginal_ptr);
        uint64_t metadata = *(uint64_t *)(orginal_ptr_wotag - 1);
        //printf("authing ptr %p with %lx\n",orginal_ptr,metadata);
        asm("autda %0, %2" : "=r"(orginal_ptr) : "0"(orginal_ptr), "r"(metadata));
        //printf("authed ptr %p\n",orginal_ptr);
        return actual_ptr;
}

uint64_t main() {
        // uint64_t context = 0x233;

        // void *ptr = buff_ptr;
        // void * signed_ptr = ptr;
        // *((uint64_t *)buff_ptr - 1) = context;
        // printf("      unsigned pointer: %p\n", ptr);

        // asm("pacda %0, %2" : "=r"(signed_ptr) : "0"(signed_ptr), "r"(context));
        // printf("        signed pointer: %p\n", signed_ptr);

        // uint64_t sz_bits = 0x9;
        // sz_bits = sz_bits << 56;
        // uint64_t * signed_ptr_with_sz = (uint64_t long) ptr + sz_bits;
        // printf("((1<<9)<<56) 0x%lx\n",sz_bits);
        // printf(" signed_ptr_with_sz pointer: %p\n", signed_ptr_with_sz);
        // asm("pacda %0, %2" : "=r"(signed_ptr_with_sz) : "0"(signed_ptr_with_sz), "r"(context));
        // printf(" signed_ptr_with_sz pointer before authenticated: %p\n", signed_ptr_with_sz);

        // uint64_t paced_ptr = signed_ptr_with_sz;

        // uint64_t sz = 1 << ((uint64_t)signed_ptr_with_sz>>56);
        // uint64_t metadata = *(uint64_t *)((((uint64_t)signed_ptr_with_sz & 0xFFFFFFFFFFFF)>>sz<<sz) - 8);
        // printf("metadata %lx \n",metadata);
        // asm("autda %0, %2" : "=r"(signed_ptr_with_sz) : "0"(signed_ptr_with_sz), "r"(metadata));
        // printf("signed_ptr_with_sz pointer after authenticated %p\n", signed_ptr_with_sz);
        // printf("deallocating signed_ptr_with_sz %lx\n",*signed_ptr_with_sz);


        // uint64_t auted_ptr;

        // // asm("and %0, %1, #0xFFFFFFFFFFFF;"
        // //     "lsr %2, %1, #55"
        // // : "=r"(auted_ptr)
        // // :"r"(paced_ptr)
        // // : "%2");
        // uint64_t actual_ptr = auted_ptr & 0xFFFFFFFFFFFF;
        // uint64_t auted_metadata = *(uint64_t *)((((uint64_t)auted_ptr & 0xFFFFFFFFFFFF)>>sz<<sz) - 8);

        // printf("paced_ptr : %lx \n",paced_ptr);
        // printf("auted_ptr : %lx \n",auted_ptr);

        // void *authenticated = signed_ptr;
        // asm("autda %0, %2" : "=r"(authenticated) : "0"(authenticated), "r"(context));
        // printf(" authenticated pointer: %p\n", authenticated);

        // //what if we use the wrong context to authenticate?
        // void *noauthenticated = signed_ptr;
        // asm("autda %0, %2" : "=r"(noauthenticated) : "0"(noauthenticated), "r"(0x666));
        // printf("aut with wrong context: %p\n", noauthenticated);

        // // what if we don't authenticate?
        // int * signed_int_ptr = signed_ptr;
        // printf("aut with wrong context: %d\n", *signed_int_ptr);



        void *ptr = buff_ptr;
        
        void * paced_ptr = pac(ptr, 9);

        return (uint64_t )auth(paced_ptr);
}



// int main(){
//     char * pac_ptr = (uint64_t)buff_ptr 
//     asm("MOV X2, 0x9");
//     asm("lsl x2, x2, #56");
//     asm("orr x1, x1, x2");
//     asm("PACIA X1, X2");
//     asm("AUTIA X1, X2");
//     printf("buff %p size 0x%x \nbuff_ptr %p\n",buff,1<<10,buff_ptr);
//     // printf("hello world\n");
// }