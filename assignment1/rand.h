#ifndef RAND_H_INCLUDED
#define RAND_H_INCLUDED

void rand_init();

unsigned long rand_uint();

unsigned long rand_uint_inclusive(int, int);

#endif // RAND_H_INCLUDED