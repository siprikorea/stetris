#include "strandom.h"

/************************************************************
 *  @brief      Constructor
 *  @retval     Nothing
 ************************************************************/
CStRandom::CStRandom()
    : m_dwState(1)
{
}

/************************************************************
 *  @brief      Seed
 *  @param[in]  dwSeed          Seed
 *  @retval     Nothing
 ************************************************************/
void CStRandom::Seed(unsigned int dwSeed)
{
    // A zero state would make the generator stick at zero
    m_dwState = dwSeed ? dwSeed : 1;
}

/************************************************************
 *  @brief      Get next value
 *  @retval     Value
 ************************************************************/
unsigned int CStRandom::Next()
{
    // Numerical Recipes linear congruential generator
    m_dwState = m_dwState * 1664525u + 1013904223u;

    // The low bits of an LCG are weak, so use the high ones
    return m_dwState >> 16;
}

/************************************************************
 *  @brief      Get next value in [nMin, nMax]
 *  @param[in]  nMin            Minimum value
 *  @param[in]  nMax            Maximum value
 *  @retval     Value
 ************************************************************/
int CStRandom::NextRange(int nMin, int nMax)
{
    if (nMax <= nMin)
        return nMin;

    return nMin + (int)(Next() % (unsigned int)(nMax - nMin + 1));
}
