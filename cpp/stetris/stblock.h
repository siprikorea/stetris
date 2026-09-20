#ifndef __STBLOCK_H__
#define __STBLOCK_H__

#include "stboard.h"
#include "stblocks.h"

class CStBlock
{
public:
    // Constructor
    CStBlock(CStBoard* pBoard, int nType);

    // Reset to the spawn position with the given type
    void Reset(int nType);
    // Take another block's shape and position
    void CopyFrom(const CStBlock& other);

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
	int GetCell(int nX, int nY);

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
    bool Fits(int nMoveX, int nMoveY, int MoveBlock[StBlocks::SIZE][StBlocks::SIZE]);

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
    int m_Block[StBlocks::SIZE][StBlocks::SIZE];
};

#endif 
