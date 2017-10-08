#include <stdio.h>
#include <time.h>
#include "mt.h"
#include "rand.h"

void rand_init() 
{
        init_genrand(time(NULL));
}

unsigned long rand_uint()
{
	unsigned long eax;
	unsigned long ebx;
	unsigned long ecx;
	unsigned long edx;
	eax = 0x01;

	__asm__ __volatile__(
		"cpuid;"
		: "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
		: "a"(eax)
	);

	unsigned long rand;
	if (ecx & 0x40000000) {
		__asm__ __volatile__(
			"rdrand %%eax"
			: "=a"(rand)
		);
	} else {
		rand = genrand_int32();
	}

	return rand;
}

unsigned long rand_uint_inclusive(int lower, int upper)
{
	unsigned long output = (rand_uint() % (upper - lower + 1)) + lower;
	return output;
}