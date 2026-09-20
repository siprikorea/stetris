#include "stboard.h"

// Definitions for the in class constants, needed before C++17
const int CStBoard::X_SIZE;
const int CStBoard::Y_SIZE;

/************************************************************
 *	@brief		Constructor
 *	@retval		Nothing
 ************************************************************/
CStBoard::CStBoard()
{
    // Clear
    Clear();
}

/************************************************************
 *	@brief		Clear
 *	@retval		Nothing
 ************************************************************/
void CStBoard::Clear()
{
	for (int nBoardY = 0; nBoardY < Y_SIZE; nBoardY++)
	{
		for (int nBoardX = 0; nBoardX < X_SIZE; nBoardX++)
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
    return X_SIZE;
}

/************************************************************
 *	@brief		Get Y size
 *	@retval		Nothing
 ************************************************************/
int CStBoard::GetYSize()
{
    return Y_SIZE;
}

/************************************************************
 *	@brief		Get value
 *	@retval		Nothing
 ************************************************************/
int CStBoard::GetValue(int nX, int nY)
{
	if (nX < 0 || nX >= X_SIZE)
		return 0;

	if (nY < 0 || nY >= Y_SIZE)
		return 0;

    return m_Board[nY][nX];
}

/************************************************************
 *	@brief		Set value
 *	@retval		Nothing
 ************************************************************/
void CStBoard::SetValue(int nX, int nY, int nValue)
{
	if (nX < 0 || nX >= X_SIZE)
		return;

	if (nY < 0 || nY >= Y_SIZE)
		return;

    m_Board[nY][nX] = nValue;
}
