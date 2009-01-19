#include "e_set.h"

Set::Set(int t)
{
    int i;
    const int t2 = t/32;

    S = new unsigned int[t2];

    for (i = 0; i < t2; i++)
        S[i] = 0;
}

Set::~Set()
{
    delete[] S;
}
