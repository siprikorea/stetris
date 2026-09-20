#include "stboard.h"

/************************************************************
 *	@brief		Constructor
 *	@retval		Nothing
 ************************************************************/
CStBoard::CStBoard()
{
    // X Size
    m_XSize = ST_MAX_BOARD_X;
    // Y Size
    m_YSize = ST_MAX_BOARD_Y;
    // Clear
    Clear();
}

/************************************************************
 *	@brief		Clear
 *	@retval		Nothing
 ************************************************************/
void CStBoard::Clear()
{
	for (int nBoardY = 0; nBoardY < m_YSize; nBoardY++)
	{
		for (int nBoardX = 0; nBoardX < m_XSize; nBoardX++)
		{
			m_Board[nBoardY][nBoardX] = 0;
		}
	}
}

/************************************************************
 *	@brief		Get X size
 *	@retval		Nothing
 ************************************************************/
int CStBoard::GetXSize()
{
    return m_XSize;
}

/************************************************************
 *	@brief		Get Y size
 *	@retval		Nothing
 ************************************************************/
int CStBoard::GetYSize()
{
    return m_YSize;
}

/************************************************************
 *	@brief		Get value
 *	@retval		Nothing
 ************************************************************/
int CStBoard::GetValue(int nX, int nY)
{
	if (nX < 0 || nX >= ST_MAX_BOARD_X)
		return 0;

	if (nY < 0 || nY >= ST_MAX_BOARD_Y)
		return 0;

    return m_Board[nY][nX];
}

/************************************************************
 *	@brief		Set value
 *	@retval		Nothing
 ************************************************************/
void CStBoard::SetValue(int nX, int nY, int nValue)
{
	if (nX < 0 || nX >= ST_MAX_BOARD_X)
		return;

	if (nY < 0 || nY >= ST_MAX_BOARD_Y)
		return;

    m_Board[nY][nX] = nValue;
}
