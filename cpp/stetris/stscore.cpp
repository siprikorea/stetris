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
void CStScore::Add(unsigned int dwScore)
{
	m_dwScore += dwScore;
}

/************************************************************
 *	@brief		Set score
 *	@retval		Nothing
 ************************************************************/
void CStScore::Set(unsigned int dwScore)
{
	m_dwScore = dwScore;
}

/************************************************************
 *	@brief		Get score
 *	@retval		Nothing
 ************************************************************/
unsigned int CStScore::Get()
{
	return m_dwScore;
}
