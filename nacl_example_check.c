void* sxfi_check_deref(void *addr) __attribute__((always_inline)) {
	 asm(
	//"or $0x0, %%r15;"
 	"or $0x0, %0;"
	"or %0, %0;"
	: "=r"(addr)
	: "0"(addr)
   );
	return addr;
}