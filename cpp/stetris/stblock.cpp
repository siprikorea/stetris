#include <string.h>
#include "stblock.h"
#include "stblocks.h"

/************************************************************
 *  @brief      Constructor
 *  @param[in]  pBoard          Board
 *  @param[in]  nType           Block type (1 ~ StBlocks::COUNT)
 *  @retval     Nothing
 ************************************************************/
CStBlock::CStBlock(CStBoard* pBoard, int nType)
    : m_pBoard(pBoard)
{
    Reset(nType);
}

/************************************************************
 *  @brief      Reset to the spawn position with the given type
 *  @param[in]  nType           Block type (1 ~ StBlocks::COUNT)
 *  @retval     Nothing
 ************************************************************/
void CStBlock::Reset(int nType)
{
    // Clamp the type so an out of range value cannot index past the table
    if (nType < 1 || nType > StBlocks::COUNT)
        nType = 1;

    // Type
    m_Type = nType;
    // X Size
    m_XSize = g_StBlocks[m_Type-1].x_size;
    // Y Size
    m_YSize = g_StBlocks[m_Type-1].y_size;
    // X Position
    m_XPos = (m_pBoard->GetXSize() / 2) - (m_XSize / 2);
    // Y Position
    m_YPos = 0;
    // Rotation
    m_Rotation = 0;
    // Block
    memcpy(m_Block, &g_StBlocks[m_Type-1].block[m_Rotation], sizeof(m_Block));
}

/************************************************************
 *  @brief      Take another block's shape and position
 *  @param[in]  other           Block to copy
 *  @retval     Nothing
 ************************************************************/
void CStBlock::CopyFrom(const CStBlock& other)
{
    m_Type = other.m_Type;
    m_XSize = other.m_XSize;
    m_YSize = other.m_YSize;
    m_XPos = other.m_XPos;
    m_YPos = other.m_YPos;
    m_Rotation = other.m_Rotation;
    memcpy(m_Block, other.m_Block, sizeof(m_Block));
}

/************************************************************
 *  @brief      Get type
 *  @retval     Type
 ************************************************************/
int CStBlock::GetType()
{
    return m_Type;
}

/************************************************************
 *  @brief      Get X size
 *  @retval     X size
 ************************************************************/
int CStBlock::GetXSize()
{
    return m_XSize;
}

/************************************************************
 *  @brief      Get Y size
 *  @retval     Y size
 ************************************************************/
int CStBlock::GetYSize()
{
    return m_YSize;
}

/************************************************************
 *  @brief      Get X position
 *  @retval     X position
 ************************************************************/
int CStBlock::GetXPos()
{
    return m_XPos;
}

/************************************************************
 *  @brief      Get Y position
 *  @retval     Y position
 ************************************************************/
int CStBlock::GetYPos()
{
    return m_YPos;
}

/************************************************************
 *  @brief      Get block
 *  @param[in]  nX              X position in the block
 *  @param[in]  nY              Y position in the block
 *  @retval     Cell value (0 = empty)
 ************************************************************/
int CStBlock::GetCell(int nX, int nY)
{
    if (nX < 0 || nX >= StBlocks::SIZE)
        return 0;

    if (nY < 0 || nY >= StBlocks::SIZE)
        return 0;

    return m_Block[nY][nX];
}

/************************************************************
 *  @brief      Rotate
 *  @retval     true            Rotated
 *  @retval     false           Blocked
 ************************************************************/
bool CStBlock::Rotate()
{
    // Set temp rotation
    int nTempRotation = (m_Rotation + 1) % StBlocks::ROTATIONS;
    // Set temp block
    int TempBlock[StBlocks::SIZE][StBlocks::SIZE];
    memcpy(TempBlock, &g_StBlocks[m_Type-1].block[nTempRotation], sizeof(TempBlock));

    // Wall kick - try in place first, then nudge left and right.
    // Without this a block can never be rotated next to a wall.
    static const int nKicks[] = { 0, -1, 1, -2, 2 };

    for (int nKick = 0; nKick < (int)(sizeof(nKicks) / sizeof(nKicks[0])); nKick++)
    {
        int nTempXPos = m_XPos + nKicks[nKick];

        // Check bounds
        if (!Fits(nTempXPos, m_YPos, TempBlock))
        {
            continue;
        }

        // Set current rotation
        m_Rotation = nTempRotation;
        // Set current X position
        m_XPos = nTempXPos;
        // Set current block
        memcpy(m_Block, TempBlock, sizeof(m_Block));
        return true;
    }

    return false;
}

/************************************************************
 *  @brief      Move left
 *  @retval     true            Moved
 *  @retval     false           Blocked
 ************************************************************/
bool CStBlock::MoveLeft()
{
    // Check bounds
    if (Fits(m_XPos - 1, m_YPos, m_Block))
    {
        // Set current X position
        m_XPos = m_XPos - 1;
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************
 *  @brief      Move right
 *  @retval     true            Moved
 *  @retval     false           Blocked
 ************************************************************/
bool CStBlock::MoveRight()
{
    // Check bounds
    if (Fits(m_XPos + 1, m_YPos, m_Block))
    {
        // Set current X position
        m_XPos = m_XPos + 1;
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************
 *  @brief      Move down
 *  @retval     true            Moved
 *  @retval     false           Landed
 ************************************************************/
bool CStBlock::MoveDown()
{
    // Check bounds
    if (Fits(m_XPos, m_YPos + 1, m_Block))
    {
        // Set current Y position
        m_YPos = m_YPos + 1;
        return true;
    }
    else
    {
        return false;
    }
}

/************************************************************
 *  @brief      Drop
 *  @retval     Number of cells the block fell
 ************************************************************/
int CStBlock::Drop()
{
    int nDistance = 0;

    while (MoveDown())
    {
        nDistance++;
    }

    return nDistance;
}

/************************************************************
 *  @brief      Check if the block fits at its current position
 *  @retval     true            Fits
 *  @retval     false           Overlaps the board or is out of bounds
 ************************************************************/
bool CStBlock::CanPlace()
{
    return Fits(m_XPos, m_YPos, m_Block);
}

/************************************************************
 *  @brief      Check bounds
 *  @param[in]  nMoveX          X position to test
 *  @param[in]  nMoveY          Y position to test
 *  @param[in]  MoveBlock       Block shape to test
 *  @retval     true            Fits
 *  @retval     false           Does not fit
 ************************************************************/
bool CStBlock::Fits(int nMoveX, int nMoveY, int MoveBlock[StBlocks::SIZE][StBlocks::SIZE])
{
    // Get board size
    int nBoardXSize = m_pBoard->GetXSize();
    int nBoardYSize = m_pBoard->GetYSize();

    // Scan the whole shape, not just m_XSize * m_YSize - the shape being
    // tested may be a rotation that reaches further than the current one
    for (int nBlockY = 0; nBlockY < StBlocks::SIZE; nBlockY++)
    {
        for (int nBlockX = 0; nBlockX < StBlocks::SIZE; nBlockX++)
        {
            // Check if block is empty
            if (!MoveBlock[nBlockY][nBlockX])
            {
                continue;
            }

            // Check X position of block
            if ((nMoveX + nBlockX) < 0 || (nMoveX + nBlockX) >= nBoardXSize)
            {
                return false;
            }

            // Check Y position of block
            if ((nMoveY + nBlockY) < 0 || (nMoveY + nBlockY) >= nBoardYSize)
            {
                return false;
            }

            // Check if block is already exist
            if (m_pBoard->GetValue(nMoveX + nBlockX, nMoveY + nBlockY))
            {
                return false;
            }
        }
    }

    return true;
}
