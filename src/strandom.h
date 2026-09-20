#ifndef __STRANDOM_H__
#define __STRANDOM_H__

class CStRandom
{
public:
    // Constructor
    CStRandom();

    // Seed
    void Seed(unsigned int dwSeed);
    // Get next value
    unsigned int Next();
    // Get next value in [nMin, nMax]
    int NextRange(int nMin, int nMax);

private:
    // State
    unsigned int m_dwState;
};

#endif
