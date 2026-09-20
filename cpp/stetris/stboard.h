#ifndef __STBOARD_H__
#define __STBOARD_H__

#define ST_MAX_BOARD_X 10
#define ST_MAX_BOARD_Y 20

class CStBoard
{
public:
    // Constructor
    CStBoard();

    // Clear
    void Clear();

    // Get X size
    int GetXSize();
    // Get Y size
    int GetYSize();
    // Get value
    int GetValue(int nX, int nY);
    // Set value
    void SetValue(int nX, int nY, int nValue);

private:
    // X Size
    int m_XSize;
    // Y Size
    int m_YSize;
    // Board
    int m_Board[ST_MAX_BOARD_Y][ST_MAX_BOARD_X];
};

#endif
