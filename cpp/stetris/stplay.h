#ifndef __STPLAY_H__
#define __STPLAY_H__

#include "stblock.h"
#include "stboard.h"
#include "stscore.h"
#include "strandom.h"

// Game state
enum class ST_STATE
{
    PLAYING,
    PAUSED,
    GAMEOVER
};

class CStPlay
{
public:
    // Constructor
    CStPlay();

    // Start a new game
    void NewGame(unsigned int dwSeed);

    // Move left
    bool MoveLeft();
    // Move right
    bool MoveRight();
    // Rotate
    bool Rotate();
    // Soft drop, moves the block down one cell
    bool SoftDrop();
    // Hard drop, drops the block and locks it
    bool HardDrop();

    // Advance the game by the elapsed time
    void Tick(unsigned int dwElapsedMilliSec);

    // Set pause
    void SetPause(bool bPause);
    // Toggle pause
    void TogglePause();

    // Get state
    ST_STATE GetState();
    // Is playing
    bool IsPlaying();
    // Is paused
    bool IsPaused();
    // Is game over
    bool IsGameOver();

	// Get board
	CStBoard* GetBoard();
	// Get current block
	CStBlock* GetCurrentBlock();
	// Get next block
	CStBlock* GetNextBlock();
	// Get score
	CStScore* GetScore();
	// Get high score
	CStScore* GetHighScore();
    // Get level
    int GetLevel();
    // Get cleared line count
    int GetLines();
    // Get the current fall interval in milliseconds
    unsigned int GetFallInterval();

private:
    // Apply one step of gravity
    void ApplyGravity();
    // Lock the current block and bring in the next one
    void LockBlock();
	// Set block to board
	void SetBlockToBoard();
	// Clear complete lines, returns how many were cleared
	int ClearCompleteLine();
	// Change block
	void ChangeBlock();
    // Pick a random block type
    int NextBlockType();
    // Update high score from the current score
    void UpdateHighScore();

    // Board - declared before the blocks, which take its address
    CStBoard m_Board;
    // Current Block
    CStBlock m_CurrentBlock;
    // Next Block
    CStBlock m_NextBlock;
	// Score
	CStScore m_Score;
	// High Score
	CStScore m_HighScore;
    // Random generator
    CStRandom m_Random;
    // State
    ST_STATE m_State;
    // Cleared line count
    int m_nLines;
    // Time accumulated towards the next fall, in milliseconds
    unsigned int m_dwFallTimer;
};

#endif
