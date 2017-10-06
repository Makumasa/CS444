#ifndef MT_H_INCLUDED
#define MT_H_INCLUDED

void init_genrand(unsigned long s);

unsigned long genrand_int32(void);

long genrand_int31(void);

double genrand_real1(void);

double genrand_real2(void);

double genrand_real3(void);

double genrand_res53(void);

#endif // MT_H_INCLUDED
