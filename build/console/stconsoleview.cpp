#include <stdio.h>
#include <string.h>
#include "stconsoleview.h"

// Screen control
#define ST_ESC_HIDE_CURSOR      "\033[?25l"
#define ST_ESC_SHOW_CURSOR      "\033[?25h"
#define ST_ESC_CLEAR_SCREEN     "\033[2J"
#define ST_ESC_HOME             "\033[H"
#define ST_ESC_CLEAR_LINE       "\033[K"
#define ST_ESC_RESET            "\033[0m"

// Cell
#define ST_CELL_FILLED          "[]"
#define ST_CELL_EMPTY           " ."

// Side panel column, one based
#define ST_SIDE_COL             28

/************************************************************
 *  @brief      Color of the block type
 *  @param[in]  nType           Block type (1 ~ 7)
 *  @retval     Escape sequence
 ************************************************************/
static const char* StGetColor(int nType)
{
    switch (nType)
    {
    case 1:  return "\033[1;34m";   // J - blue
    case 2:  return "\033[1;37m";   // L - white
    case 3:  return "\033[1;33m";   // O - yellow
    case 4:  return "\033[1;32m";   // S - green
    case 5:  return "\033[1;35m";   // T - magenta
    case 6:  return "\033[1;31m";   // Z - red
    case 7:  return "\033[1;36m";   // I - cyan
    default: return "\033[0;90m";   // empty - gray
    }
}

/************************************************************
 *  @brief      Constructor
 *  @retval     Nothing
 ************************************************************/
CStConsoleView::CStConsoleView()
    : m_nScreenLen(0)
{
    m_szScreen[0] = '\0';
}

/************************************************************
 *  @brief      Enter screen
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::EnterScreen()
{
    fputs(ST_ESC_HIDE_CURSOR ST_ESC_CLEAR_SCREEN ST_ESC_HOME, stdout);
    fflush(stdout);
}

/************************************************************
 *  @brief      Leave screen
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::LeaveScreen()
{
    fputs(ST_ESC_RESET ST_ESC_SHOW_CURSOR "\n", stdout);
    fflush(stdout);
}

/************************************************************
 *  @brief      Put string to the screen buffer
 *  @param[in]  pszText         Text
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::PutString(const char* pszText)
{
    int nLen = (int)strlen(pszText);

    if (m_nScreenLen + nLen >= (int)sizeof(m_szScreen))
        return;

    memcpy(m_szScreen + m_nScreenLen, pszText, nLen);
    m_nScreenLen += nLen;
    m_szScreen[m_nScreenLen] = '\0';
}

/************************************************************
 *  @brief      Put one cell to the screen buffer
 *  @param[in]  nType           Block type (0 = empty)
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::PutCell(int nType)
{
    PutString(StGetColor(nType));
    PutString(nType ? ST_CELL_FILLED : ST_CELL_EMPTY);
    PutString(ST_ESC_RESET);
}

/************************************************************
 *  @brief      Render one frame
 *  @param[in]  play            Game to render
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::Render(CStPlay& play)
{
    // Reset screen buffer
    m_nScreenLen = 0;
    m_szScreen[0] = '\0';

    // Move cursor to the top left without clearing (avoids flicker)
    PutString(ST_ESC_HOME);

    // Draw board with the side panel
    DrawBoard(play);
    // Draw help
    DrawHelp();

    // Flush the whole frame at once
    fwrite(m_szScreen, 1, m_nScreenLen, stdout);
    fflush(stdout);
}

/************************************************************
 *  @brief      Draw board with the side panel
 *  @param[in]  play            Game to render
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::DrawBoard(CStPlay& play)
{
    CStBoard* pBoard = play.GetBoard();
    CStBlock* pBlock = play.GetCurrentBlock();

    int nXSize = pBoard->GetXSize();
    int nYSize = pBoard->GetYSize();

    // Merge the board and the current block into one buffer
    int Screen[ST_MAX_BOARD_Y][ST_MAX_BOARD_X];
    for (int nBoardY = 0; nBoardY < nYSize; nBoardY++)
    {
        for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
        {
            Screen[nBoardY][nBoardX] = pBoard->GetValue(nBoardX, nBoardY);
        }
    }

    // A block that is already locked into the board is drawn from the
    // board, so only draw the falling one while the game is running
    if (!play.IsGameOver())
    {
        for (int nBlockY = 0; nBlockY < pBlock->GetYSize(); nBlockY++)
        {
            for (int nBlockX = 0; nBlockX < pBlock->GetXSize(); nBlockX++)
            {
                if (!pBlock->GetBlock(nBlockX, nBlockY))
                    continue;

                int nX = pBlock->GetXPos() + nBlockX;
                int nY = pBlock->GetYPos() + nBlockY;

                if (nX < 0 || nX >= nXSize || nY < 0 || nY >= nYSize)
                    continue;

                Screen[nY][nX] = pBlock->GetType();
            }
        }
    }

    char szLine[256];

    // Title
    PutString("\033[1;37m       S T E T R I S\033[0m" ST_ESC_CLEAR_LINE "\n");

    // Top frame
    PutString("  +");
    for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
        PutString("--");
    PutString("+" ST_ESC_CLEAR_LINE "\n");

    // Board
    for (int nBoardY = 0; nBoardY < nYSize; nBoardY++)
    {
        PutString("  |");
        for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
            PutCell(Screen[nBoardY][nBoardX]);
        PutString("|");

        // Side panel on the same line
        snprintf(szLine, sizeof(szLine), "\033[%dG", ST_SIDE_COL);
        PutString(szLine);
        DrawSide(play, nBoardY);

        PutString(ST_ESC_CLEAR_LINE "\n");
    }

    // Bottom frame
    PutString("  +");
    for (int nBoardX = 0; nBoardX < nXSize; nBoardX++)
        PutString("--");
    PutString("+" ST_ESC_CLEAR_LINE "\n");
}

/************************************************************
 *  @brief      Draw one line of the side panel
 *  @param[in]  play            Game to render
 *  @param[in]  nLine           Line index inside the board area
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::DrawSide(CStPlay& play, int nLine)
{
    char szLine[256];

    switch (nLine)
    {
    case 0:
        PutString("\033[1;37mNEXT\033[0m");
        break;

    case 1:
    case 2:
    case 3:
    case 4:
        {
            // The preview is drawn at the origin of the panel, the spawn
            // position of the next block is irrelevant here
            CStBlock* pNext = play.GetNextBlock();
            int nBlockY = nLine - 1;
            for (int nBlockX = 0; nBlockX < ST_MAX_BLOCK_X; nBlockX++)
                PutCell(pNext->GetBlock(nBlockX, nBlockY) ? pNext->GetType() : 0);
        }
        break;

    case 6:
        PutString("\033[1;37mSCORE\033[0m");
        break;

    case 7:
        snprintf(szLine, sizeof(szLine), "%u", play.GetScore()->GetScore());
        PutString(szLine);
        break;

    case 9:
        PutString("\033[1;37mHIGH SCORE\033[0m");
        break;

    case 10:
        snprintf(szLine, sizeof(szLine), "%u", play.GetHighScore()->GetScore());
        PutString(szLine);
        break;

    case 12:
        PutString("\033[1;37mLEVEL\033[0m");
        break;

    case 13:
        snprintf(szLine, sizeof(szLine), "%d", play.GetLevel());
        PutString(szLine);
        break;

    case 15:
        PutString("\033[1;37mLINES\033[0m");
        break;

    case 16:
        snprintf(szLine, sizeof(szLine), "%d", play.GetLines());
        PutString(szLine);
        break;

    case 18:
        if (play.IsGameOver())
            PutString("\033[1;31mGAME OVER\033[0m");
        else if (play.IsPaused())
            PutString("\033[1;33mPAUSED\033[0m");
        break;

    case 19:
        if (play.IsGameOver())
            PutString("\033[1;31mpress R to restart\033[0m");
        break;

    default:
        break;
    }
}

/************************************************************
 *  @brief      Draw help
 *  @retval     Nothing
 ************************************************************/
void CStConsoleView::DrawHelp()
{
    PutString(ST_ESC_CLEAR_LINE "\n");
    PutString("  \033[0;90mLeft/Right: move   Up: rotate   Down: soft drop\033[0m" ST_ESC_CLEAR_LINE "\n");
    PutString("  \033[0;90mSpace: hard drop   P: pause     R: restart   Q: quit\033[0m" ST_ESC_CLEAR_LINE "\n");
}
