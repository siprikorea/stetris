#ifndef __STBOARD_H__
#define __STBOARD_H__

class CStBoard
{
public:
    // Width of the board in cells
    static const int X_SIZE = 10;
    // Height of the board in cells
    static const int Y_SIZE = 20;

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
    // Board
    int m_Board[Y_SIZE][X_SIZE];
};

#endif
