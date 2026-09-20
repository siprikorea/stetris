//
// Drives the game logic with no UI linked in at all. If this file ever needs
// a header from a UI directory, the layers have stopped being separate.
//
#include <stdio.h>
#include "stplay.h"

// Failure count
static int g_nFail = 0;

/************************************************************
 *  @brief      Report the result of one check
 *  @param[in]  pszName         Check name
 *  @param[in]  bPass           Result
 *  @param[in]  pszDetail       Detail shown next to the name
 *  @retval     Nothing
 ************************************************************/
static void StCheck(const char* pszName, bool bPass, const char* pszDetail)
{
    printf("  %-14s %-28s %s\n", pszName, pszDetail, bPass ? "ok" : "FAIL");

    if (!bPass)
        g_nFail++;
}

/************************************************************
 *  @brief      Play a whole game out with hard drops only
 *  @param[in]  dwSeed          Random seed
 *  @retval     Final score
 ************************************************************/
static unsigned int StPlayOut(unsigned int dwSeed)
{
    CStPlay play;
    play.NewGame(dwSeed);

    // The guard keeps a logic bug from hanging the test run
    int nGuard = 0;
    while (!play.IsGameOver() && nGuard++ < 100000)
    {
        play.HardDrop();
    }

    return play.GetScore()->GetScore();
}

/************************************************************
 *  @brief      The same seed must replay the same game
 *  @retval     Nothing
 ************************************************************/
static void StTestDeterminism()
{
    char szDetail[128];

    unsigned int dwFirst = StPlayOut(12345);
    unsigned int dwSecond = StPlayOut(12345);
    snprintf(szDetail, sizeof(szDetail), "seed 12345 -> %u, %u", dwFirst, dwSecond);
    StCheck("determinism", dwFirst == dwSecond, szDetail);

    unsigned int dwOther = StPlayOut(999);
    snprintf(szDetail, sizeof(szDetail), "seed 999 -> %u", dwOther);
    StCheck("variation", dwOther != dwFirst, szDetail);
}

/************************************************************
 *  @brief      Gravity must not score, only player input does
 *  @retval     Nothing
 ************************************************************/
static void StTestIdleScore()
{
    char szDetail[128];

    CStPlay play;
    play.NewGame(7);

    // Ten seconds of falling without touching a key
    for (int nStep = 0; nStep < 100; nStep++)
        play.Tick(100);

    snprintf(szDetail, sizeof(szDetail), "10s idle -> %u", play.GetScore()->GetScore());
    StCheck("idle score", play.GetScore()->GetScore() == 0, szDetail);
}

/************************************************************
 *  @brief      A paused game must not advance or accept input
 *  @retval     Nothing
 ************************************************************/
static void StTestPause()
{
    char szDetail[128];

    CStPlay play;
    play.NewGame(7);
    play.SetPause(true);

    int nYPos = play.GetCurrentBlock()->GetYPos();
    for (int nStep = 0; nStep < 50; nStep++)
        play.Tick(100);

    bool bFrozen = (play.GetCurrentBlock()->GetYPos() == nYPos);
    bool bNoInput = !play.MoveLeft() && !play.Rotate() && !play.HardDrop();

    snprintf(szDetail, sizeof(szDetail), "frozen %d, input blocked %d", bFrozen, bNoInput);
    StCheck("pause", bFrozen && bNoInput, szDetail);

    // Resuming must bring the game back
    play.SetPause(false);
    StCheck("resume", play.IsPlaying() && play.MoveLeft(), "moves again");
}

/************************************************************
 *  @brief      The game must end and stay ended
 *  @retval     Nothing
 ************************************************************/
static void StTestGameOver()
{
    char szDetail[128];

    CStPlay play;
    play.NewGame(42);

    int nDrops = 0;
    while (!play.IsGameOver() && nDrops < 100000)
    {
        play.HardDrop();
        nDrops++;
    }

    snprintf(szDetail, sizeof(szDetail), "ended after %d drops", nDrops);
    StCheck("game over", play.IsGameOver(), szDetail);

    bool bLocked = !play.HardDrop() && !play.Rotate() && !play.MoveLeft();
    StCheck("stays over", bLocked, "input ignored");

    // Pausing must not resurrect a finished game
    play.SetPause(false);
    StCheck("no revive", play.IsGameOver(), "unpause does nothing");
}

/************************************************************
 *  @brief      Clearing lines must score by the line count and the level
 *  @retval     Nothing
 ************************************************************/
static void StTestLineClear()
{
    char szDetail[128];

    CStPlay play;

    // Look for a seed that starts with the square block, so the shape that
    // lands in the gap left below is known
    unsigned int dwSeed = 1;
    while (dwSeed < 1000 && play.GetCurrentBlock()->GetType() != 3)
    {
        play.NewGame(dwSeed);
        dwSeed++;
    }

    if (play.GetCurrentBlock()->GetType() != 3)
    {
        StCheck("line clear", false, "no square block seed found");
        return;
    }

    CStBoard* pBoard = play.GetBoard();
    int nXSize = pBoard->GetXSize();
    int nYSize = pBoard->GetYSize();

    // Fill the bottom two rows except the two leftmost columns
    for (int nBoardY = nYSize - 2; nBoardY < nYSize; nBoardY++)
    {
        for (int nBoardX = 2; nBoardX < nXSize; nBoardX++)
            pBoard->SetValue(nBoardX, nBoardY, 1);
    }

    unsigned int dwBefore = play.GetScore()->GetScore();

    // Slide the square into the gap and drop it, completing both rows
    while (play.MoveLeft())
        ;
    play.HardDrop();

    unsigned int dwGained = play.GetScore()->GetScore() - dwBefore;

    snprintf(szDetail, sizeof(szDetail), "%d lines, +%u", play.GetLines(), dwGained);
    StCheck("line clear", play.GetLines() == 2, szDetail);

    // Two lines at level 1 is 300, plus 2 per cell of the hard drop
    StCheck("line score", dwGained >= 300, szDetail);

    // The rows must actually be gone
    bool bEmpty = true;
    for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
    {
        if (pBoard->GetValue(nBoardX, nYSize - 1))
            bEmpty = false;
    }
    StCheck("rows removed", bEmpty, "bottom row cleared");
}

/************************************************************
 *  @brief      The level must raise the fall speed
 *  @retval     Nothing
 ************************************************************/
static void StTestSpeed()
{
    char szDetail[128];

    CStPlay play;
    play.NewGame(7);

    snprintf(szDetail, sizeof(szDetail), "level %d -> %u ms", play.GetLevel(), play.GetFallInterval());
    StCheck("fall speed", play.GetLevel() == 1 && play.GetFallInterval() == 800, szDetail);
}

/************************************************************
 *  @brief      A new game must reset the board but keep the high score
 *  @retval     Nothing
 ************************************************************/
static void StTestRestart()
{
    char szDetail[128];

    CStPlay play;
    play.NewGame(3);

    int nGuard = 0;
    while (!play.IsGameOver() && nGuard++ < 100000)
        play.HardDrop();

    unsigned int dwHighScore = play.GetHighScore()->GetScore();

    play.NewGame(4);

    bool bReset = play.GetScore()->GetScore() == 0
        && play.GetLines() == 0
        && play.IsPlaying();
    bool bKept = play.GetHighScore()->GetScore() == dwHighScore && dwHighScore > 0;

    // The board must be empty again
    bool bEmpty = true;
    CStBoard* pBoard = play.GetBoard();
    for (int nBoardY = 0; nBoardY < pBoard->GetYSize(); nBoardY++)
    {
        for (int nBoardX = 0; nBoardX < pBoard->GetXSize(); nBoardX++)
        {
            if (pBoard->GetValue(nBoardX, nBoardY))
                bEmpty = false;
        }
    }

    snprintf(szDetail, sizeof(szDetail), "high %u kept", dwHighScore);
    StCheck("restart", bReset && bEmpty, szDetail);
    StCheck("high score", bKept, szDetail);
}

/************************************************************
 *  @brief      Rotating next to a wall must succeed through a kick
 *  @retval     Nothing
 ************************************************************/
static void StTestWallKick()
{
    CStPlay play;

    // The bar is the shape that needs the kick the most
    unsigned int dwSeed = 1;
    while (dwSeed < 1000 && play.GetCurrentBlock()->GetType() != 7)
    {
        play.NewGame(dwSeed);
        dwSeed++;
    }

    if (play.GetCurrentBlock()->GetType() != 7)
    {
        StCheck("wall kick", false, "no bar block seed found");
        return;
    }

    // Stand the bar up, then push it against the right wall and turn it flat
    play.Rotate();
    while (play.MoveRight())
        ;

    StCheck("wall kick", play.Rotate(), "bar rotates at the wall");
}

/************************************************************
 *  @brief      Main
 *  @retval     0               All checks passed
 *  @retval     1               At least one check failed
 ************************************************************/
int main()
{
    printf("stetris logic tests\n\n");

    StTestDeterminism();
    StTestIdleScore();
    StTestPause();
    StTestGameOver();
    StTestLineClear();
    StTestSpeed();
    StTestRestart();
    StTestWallKick();

    printf("\n%s\n", g_nFail ? "FAILED" : "all passed");

    return g_nFail ? 1 : 0;
}
