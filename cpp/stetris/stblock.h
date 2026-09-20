#ifndef __STBLOCK_H__
#define __STBLOCK_H__

#define ST_MAX_BLOCK_ROT 4
#define ST_MAX_BLOCK_X 4
#define ST_MAX_BLOCK_Y 4

#include "stboard.h"

class CStBlock
{
public:
    // Constructor
    CStBlock(CStBoard* pBoard, int nType);

    // Reset to the spawn position with the given type
    void Reset(int nType);

    // Get type
    int GetType();
    // Get X size
    int GetXSize();
    // Get Y size
    int GetYSize();
    // Get X position
    int GetXPos();
    // Get Y position
    int GetYPos();
	// Get block
	int GetBlock(int nX, int nY);

    // Rotate
    bool Rotate();
    // Move left
    bool MoveLeft();
    // Move right
    bool MoveRight();
    // Move down
    bool MoveDown();
    // Drop, returns the number of cells the block fell
    int Drop();

    // Check if the block fits at its current position
    bool CanPlace();
    // Check bounds
    bool CheckBounds(int nMoveX, int nMoveY, int MoveBlock[ST_MAX_BLOCK_Y][ST_MAX_BLOCK_X]);

protected:
    // Board
    CStBoard* m_pBoard;
    // Type
    int m_Type;
    // X size
    int m_XSize;
    // Y size
    int m_YSize;
    // X position
    int m_XPos;
    // Y position
    int m_YPos;
    // Rotation
    int m_Rotation;
    // Block
    int m_Block[ST_MAX_BLOCK_Y][ST_MAX_BLOCK_X];
};

#endif 
