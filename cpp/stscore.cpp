#include "stscore.h"

/************************************************************
 *	@brief		Constructor
 *	@retval		Nothing
 ************************************************************/
CStScore::CStScore()
{
    // Clear
    Clear();
}

/************************************************************
 *	@brief		Clear
 *	@retval		Nothing
 ************************************************************/
void CStScore::Clear()
{
	m_dwScore = 0;
}

/************************************************************
 *	@brief		Add score
 *	@retval		Nothing
 ************************************************************/
void CStScore::AddScore(unsigned int dwScore)
{
	m_dwScore += dwScore;
}

/************************************************************
 *	@brief		Set score
 *	@retval		Nothing
 ************************************************************/
void CStScore::SetScore(unsigned int dwScore)
{
	m_dwScore = dwScore;
}

/************************************************************
 *	@brief		Get score
 *	@retval		Nothing
 ************************************************************/
unsigned int CStScore::GetScore()
{
	return m_dwScore;
}
