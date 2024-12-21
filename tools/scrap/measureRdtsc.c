
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <x86intrin.h>

#include <unistd.h>

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

// Function to call CPUID and get information about the CPU
void cpuid(uint32_t *eax, uint32_t *ebx, uint32_t *ecx, uint32_t *edx) {
    __asm__ __volatile__(
        "cpuid"
        : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx)
        : "a" (*eax)
    );
}

int printCycles() {
    uint32_t eax = 0x16;  // CPUID function for cache and clock info
    uint32_t ebx, ecx, edx;
    
    // Call CPUID to get the base clock info
    cpuid(&eax, &ebx, &ecx, &edx);

    // Print out the frequency or base clock if available
    printf("CPUID information: EBX=0x%x, ECX=0x%x, EDX=0x%x\n", ebx, ecx, edx);

    return 0;
}


int main(){

   printCycles();

   uint64_t testSeconds = 2;
   
   uint64_t countStart = __rdtsc();
   usleep(1000000*testSeconds);
   uint64_t countEnd = __rdtsc();

   printf("cycles per second %lu\n",(countEnd-countStart + testSeconds/2)/(testSeconds));

}