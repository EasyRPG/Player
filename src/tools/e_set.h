#ifndef _H_SET
#define _H_SET

class Set
{
    private:

        unsigned int *S;

    public:

        Set(int t);
        ~Set();

        void insert(int x) {S[x >> 5] |= (1 << (x&31));}
        bool belongs(int x) {return ((S[x >> 5] >> (x&31)) & 1);}

};

#endif
