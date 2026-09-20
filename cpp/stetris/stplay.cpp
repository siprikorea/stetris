#include "stplay.h"
#include "stblocks.h"

// Score for clearing 1 ~ 4 lines at once, multiplied by the level
static const unsigned int g_dwLineScore[] = { 0, 100, 300, 500, 800 };

// Score per cell for a soft drop and a hard drop
#define ST_SOFT_DROP_SCORE      1
#define ST_HARD_DROP_SCORE      2

// Lines needed to advance one level
#define ST_LINES_PER_LEVEL      10

// Fall interval in milliseconds
#define ST_FALL_INTERVAL_BASE   800
#define ST_FALL_INTERVAL_STEP   70
#define ST_FALL_INTERVAL_MIN    100

/************************************************************
 *  @brief      Constructor
 *  @retval     Nothing
 ************************************************************/
CStPlay::CStPlay()
    : m_CurrentBlock(&m_Board, 1),
    m_NextBlock(&m_Board, 1)
{
    NewGame(1);
}

/************************************************************
 *  @brief      Start a new game
 *  @param[in]  dwSeed          Random seed
 *  @retval     Nothing
 ************************************************************/
void CStPlay::NewGame(unsigned int dwSeed)
{
    // Random generator - the caller supplies the seed, so a given seed
    // always produces the same game
    m_Random.Seed(dwSeed);

    // Board
    m_Board.Clear();
    // Score - the high score survives a new game
    m_Score.Clear();
    // State
    m_State = ST_STATE::PLAYING;
    // Cleared line count
    m_nLines = 0;
    // Fall timer
    m_dwFallTimer = 0;

    // Blocks
    m_CurrentBlock.Reset(NextBlockType());
    m_NextBlock.Reset(NextBlockType());
}

/************************************************************
 *  @brief      Move left
 *  @retval     true            Moved
 *  @retval     false           Blocked or not playing
 ************************************************************/
bool CStPlay::MoveLeft()
{
    if (!IsPlaying())
        return false;

    return m_CurrentBlock.MoveLeft();
}

/************************************************************
 *  @brief      Move right
 *  @retval     true            Moved
 *  @retval     false           Blocked or not playing
 ************************************************************/
bool CStPlay::MoveRight()
{
    if (!IsPlaying())
        return false;

    return m_CurrentBlock.MoveRight();
}

/************************************************************
 *  @brief      Rotate
 *  @retval     true            Rotated
 *  @retval     false           Blocked or not playing
 ************************************************************/
bool CStPlay::Rotate()
{
    if (!IsPlaying())
        return false;

    return m_CurrentBlock.Rotate();
}

/************************************************************
 *  @brief      Soft drop, moves the block down one cell
 *  @retval     true            Moved
 *  @retval     false           Landed or not playing
 ************************************************************/
bool CStPlay::SoftDrop()
{
    if (!IsPlaying())
        return false;

    // A manual drop restarts the fall timer, so the block does not
    // immediately fall again on the next tick
    m_dwFallTimer = 0;

    if (!m_CurrentBlock.MoveDown())
    {
        LockBlock();
        return false;
    }

    m_Score.Add(ST_SOFT_DROP_SCORE);
    UpdateHighScore();
    return true;
}

/************************************************************
 *  @brief      Hard drop, drops the block and locks it
 *  @retval     true            Dropped
 *  @retval     false           Not playing
 ************************************************************/
bool CStPlay::HardDrop()
{
    if (!IsPlaying())
        return false;

    int nDistance = m_CurrentBlock.Drop();

    m_Score.Add(ST_HARD_DROP_SCORE * (unsigned int)nDistance);
    UpdateHighScore();

    LockBlock();
    return true;
}

/************************************************************
 *  @brief      Advance the game by the elapsed time
 *  @param[in]  dwElapsedMilliSec   Elapsed time in milliseconds
 *  @retval     Nothing
 ************************************************************/
void CStPlay::Tick(unsigned int dwElapsedMilliSec)
{
    if (!IsPlaying())
        return;

    m_dwFallTimer += dwElapsedMilliSec;

    unsigned int dwInterval = GetFallInterval();

    // Catch up if more than one interval has passed
    while (m_dwFallTimer >= dwInterval && IsPlaying())
    {
        m_dwFallTimer -= dwInterval;
        ApplyGravity();
        dwInterval = GetFallInterval();
    }
}

/************************************************************
 *  @brief      Set pause
 *  @param[in]  bPause          Pause
 *  @retval     Nothing
 ************************************************************/
void CStPlay::SetPause(bool bPause)
{
    // The game cannot be paused or resumed once it is over
    if (m_State == ST_STATE::GAMEOVER)
        return;

    m_State = bPause ? ST_STATE::PAUSED : ST_STATE::PLAYING;
}

/************************************************************
 *  @brief      Toggle pause
 *  @retval     Nothing
 ************************************************************/
void CStPlay::TogglePause()
{
    SetPause(m_State == ST_STATE::PLAYING);
}

/************************************************************
 *  @brief      Get state
 *  @retval     State
 ************************************************************/
ST_STATE CStPlay::GetState()
{
    return m_State;
}

/************************************************************
 *  @brief      Is playing
 *  @retval     true / false
 ************************************************************/
bool CStPlay::IsPlaying()
{
    return m_State == ST_STATE::PLAYING;
}

/************************************************************
 *  @brief      Is paused
 *  @retval     true / false
 ************************************************************/
bool CStPlay::IsPaused()
{
    return m_State == ST_STATE::PAUSED;
}

/************************************************************
 *  @brief      Is game over
 *  @retval     true / false
 ************************************************************/
bool CStPlay::IsGameOver()
{
    return m_State == ST_STATE::GAMEOVER;
}

/************************************************************
 *  @brief      Get board
 *  @retval     Board
 ************************************************************/
CStBoard* CStPlay::GetBoard()
{
    return &m_Board;
}

/************************************************************
 *  @brief      Get current block
 *  @retval     Current block
 ************************************************************/
CStBlock* CStPlay::GetCurrentBlock()
{
    return &m_CurrentBlock;
}

/************************************************************
 *  @brief      Get next block
 *  @retval     Next block
 ************************************************************/
CStBlock* CStPlay::GetNextBlock()
{
    return &m_NextBlock;
}

/************************************************************
 *  @brief      Get score
 *  @retval     Score
 ************************************************************/
CStScore* CStPlay::GetScore()
{
    return &m_Score;
}

/************************************************************
 *  @brief      Get high score
 *  @retval     High score
 ************************************************************/
CStScore* CStPlay::GetHighScore()
{
    return &m_HighScore;
}

/************************************************************
 *  @brief      Get level
 *  @retval     Level, starting at 1
 ************************************************************/
int CStPlay::GetLevel()
{
    return (m_nLines / ST_LINES_PER_LEVEL) + 1;
}

/************************************************************
 *  @brief      Get cleared line count
 *  @retval     Cleared line count
 ************************************************************/
int CStPlay::GetLines()
{
    return m_nLines;
}

/************************************************************
 *  @brief      Get the current fall interval in milliseconds
 *  @retval     Interval in milliseconds
 ************************************************************/
unsigned int CStPlay::GetFallInterval()
{
    int nInterval = ST_FALL_INTERVAL_BASE - (GetLevel() - 1) * ST_FALL_INTERVAL_STEP;

    if (nInterval < ST_FALL_INTERVAL_MIN)
        nInterval = ST_FALL_INTERVAL_MIN;

    return (unsigned int)nInterval;
}

/************************************************************
 *  @brief      Apply one step of gravity
 *  @retval     Nothing
 ************************************************************/
void CStPlay::ApplyGravity()
{
    // Falling on its own scores nothing
    if (!m_CurrentBlock.MoveDown())
    {
        LockBlock();
    }
}

/************************************************************
 *  @brief      Lock the current block and bring in the next one
 *  @retval     Nothing
 ************************************************************/
void CStPlay::LockBlock()
{
    // Set block to board
    SetBlockToBoard();

    // Clear complete line
    int nCleared = ClearCompleteLine();
    if (nCleared > 0)
    {
        m_nLines += nCleared;

        if (nCleared > 4)
            nCleared = 4;

        // Score after the level has been updated by the new lines
        m_Score.Add(g_dwLineScore[nCleared] * (unsigned int)GetLevel());
        UpdateHighScore();
    }

    // Change block
    ChangeBlock();

    // Fall timer
    m_dwFallTimer = 0;
}

/************************************************************
 *  @brief      Set block to board
 *  @retval     Nothing
 ************************************************************/
void CStPlay::SetBlockToBoard()
{
    // Get block size
    int nXSize = m_CurrentBlock.GetXSize();
    int nYSize = m_CurrentBlock.GetYSize();

    // Set block to board
    for (int nBlockY = 0; nBlockY < nYSize; nBlockY++)
    {
        for (int nBlockX = 0; nBlockX < nXSize; nBlockX++)
        {
            if (m_CurrentBlock.GetCell(nBlockX, nBlockY))
            {
                m_Board.SetValue(m_CurrentBlock.GetXPos() + nBlockX, m_CurrentBlock.GetYPos() + nBlockY, m_CurrentBlock.GetType());
            }
        }
    }
}

/************************************************************
 *  @brief      Clear complete lines
 *  @retval     Number of cleared lines
 ************************************************************/
int CStPlay::ClearCompleteLine()
{
    // Get board size
    int nXSize = m_Board.GetXSize();
    int nYSize = m_Board.GetYSize();

    int nCleared = 0;

    // Check if a line is completed on board
    for (int nBoardY = nYSize -1; nBoardY >= 0; )
    {
        // Count block in one line
        int nBlockCount = 0;
        for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
        {
            if (m_Board.GetValue(nBoardX, nBoardY))
            {
                nBlockCount++;
            }
        }

        // If block is completed
        if (nBlockCount == nXSize)
        {
            // Move blocks from complete block - 1 to first block
            for (int nMoveBoardY = nBoardY; nMoveBoardY > 0; nMoveBoardY--)
            {
                for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
                {
                    int nValue = m_Board.GetValue(nBoardX, nMoveBoardY - 1);
                    m_Board.SetValue(nBoardX, nMoveBoardY, nValue);
                }
            }

            // Clear first line
            for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
            {
                m_Board.SetValue(nBoardX, 0, 0);
            }

            nCleared++;
        }
        // If block is not completed
        else
        {
            // Check next line
            nBoardY--;
        }
    }

    return nCleared;
}

/************************************************************
 *  @brief      Change block
 *  @retval     Nothing
 ************************************************************/
void CStPlay::ChangeBlock()
{
    // Set current block
    m_CurrentBlock.CopyFrom(m_NextBlock);
    // Set next block
    m_NextBlock.Reset(NextBlockType());

    // The new block does not fit at its spawn position, so the stack
    // has reached the top
    if (!m_CurrentBlock.CanPlace())
    {
        m_State = ST_STATE::GAMEOVER;
    }
}

/************************************************************
 *  @brief      Pick a random block type
 *  @retval     Block type (1 ~ ST_MAX_BLOCK_CNT)
 ************************************************************/
int CStPlay::NextBlockType()
{
    return m_Random.NextRange(1, ST_MAX_BLOCK_CNT);
}

/************************************************************
 *  @brief      Update high score from the current score
 *  @retval     Nothing
 ************************************************************/
void CStPlay::UpdateHighScore()
{
    if (m_HighScore.Get() < m_Score.Get())
        m_HighScore.Set(m_Score.Get());
}
