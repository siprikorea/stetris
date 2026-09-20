#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "stconsoleview.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#endif

// Keys
#define ST_KEY_NONE         0
#define ST_KEY_LEFT         1
#define ST_KEY_RIGHT        2
#define ST_KEY_DOWN         3
#define ST_KEY_UP           4
#define ST_KEY_DROP         5
#define ST_KEY_PAUSE        6
#define ST_KEY_RESTART      7
#define ST_KEY_QUIT         8

// Input poll interval in milliseconds
#define ST_POLL_INTERVAL    20

#ifndef _WIN32
// Saved terminal attributes
static struct termios g_OldTerm;
// Terminal is in raw mode
static bool g_bRawMode = false;
#endif

/************************************************************
 *  @brief      Enter raw mode
 *  @retval     Nothing
 ************************************************************/
static void StEnterRawMode()
{
#ifndef _WIN32
    if (tcgetattr(STDIN_FILENO, &g_OldTerm) != 0)
        return;

    struct termios raw = g_OldTerm;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    g_bRawMode = true;
#endif
}

/************************************************************
 *  @brief      Leave raw mode
 *  @retval     Nothing
 ************************************************************/
static void StLeaveRawMode()
{
#ifndef _WIN32
    if (!g_bRawMode)
        return;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_OldTerm);
    g_bRawMode = false;
#endif
}

/************************************************************
 *  @brief      Get current time in milliseconds
 *  @retval     Milliseconds
 ************************************************************/
static long long StGetTick()
{
#ifdef _WIN32
    return (long long)GetTickCount64();
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;
#endif
}

/************************************************************
 *  @brief      Sleep in milliseconds
 *  @param[in]  nMilliSec       Milliseconds
 *  @retval     Nothing
 ************************************************************/
static void StSleep(int nMilliSec)
{
#ifdef _WIN32
    Sleep(nMilliSec);
#else
    struct timespec ts;
    ts.tv_sec = nMilliSec / 1000;
    ts.tv_nsec = (long)(nMilliSec % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

/************************************************************
 *  @brief      Get a key without blocking
 *  @retval     Key code (ST_KEY_NONE when no key is pending)
 ************************************************************/
static int StGetKey()
{
#ifdef _WIN32
    if (!_kbhit())
        return ST_KEY_NONE;

    int ch = _getch();
    if (ch == 0 || ch == 0xE0)
    {
        switch (_getch())
        {
        case 75: return ST_KEY_LEFT;
        case 77: return ST_KEY_RIGHT;
        case 80: return ST_KEY_DOWN;
        case 72: return ST_KEY_UP;
        }
        return ST_KEY_NONE;
    }
#else
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0)
        return ST_KEY_NONE;

    unsigned char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) != 1)
        return ST_KEY_NONE;

    // Arrow keys arrive as "ESC [ A" ~ "ESC [ D"
    if (ch == 27)
    {
        unsigned char seq[2] = { 0, 0 };

        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0)
            return ST_KEY_NONE;
        if (read(STDIN_FILENO, &seq[0], 1) != 1 || seq[0] != '[')
            return ST_KEY_NONE;
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
            return ST_KEY_NONE;

        switch (seq[1])
        {
        case 'A': return ST_KEY_UP;
        case 'B': return ST_KEY_DOWN;
        case 'C': return ST_KEY_RIGHT;
        case 'D': return ST_KEY_LEFT;
        }
        return ST_KEY_NONE;
    }
#endif

    switch (ch)
    {
    case ' ':               return ST_KEY_DROP;
    case 'p': case 'P':     return ST_KEY_PAUSE;
    case 'r': case 'R':     return ST_KEY_RESTART;
    case 'q': case 'Q':     return ST_KEY_QUIT;
    // WASD as an alternative to the arrow keys
    case 'a': case 'A':     return ST_KEY_LEFT;
    case 'd': case 'D':     return ST_KEY_RIGHT;
    case 's': case 'S':     return ST_KEY_DOWN;
    case 'w': case 'W':     return ST_KEY_UP;
    }

    return ST_KEY_NONE;
}

/************************************************************
 *  @brief      Path of the high score file
 *  @retval     Path
 ************************************************************/
static const char* StGetHighScorePath()
{
    static char szPath[1024];

#ifdef _WIN32
    const char* pszHome = getenv("APPDATA");
#else
    const char* pszHome = getenv("HOME");
#endif

    if (pszHome && *pszHome)
        snprintf(szPath, sizeof(szPath), "%s/.stetris_highscore", pszHome);
    else
        snprintf(szPath, sizeof(szPath), ".stetris_highscore");

    return szPath;
}

/************************************************************
 *  @brief      Load high score
 *  @retval     High score
 ************************************************************/
static unsigned int StLoadHighScore()
{
    FILE* fp = fopen(StGetHighScorePath(), "r");
    if (!fp)
        return 0;

    unsigned int dwHighScore = 0;
    if (fscanf(fp, "%u", &dwHighScore) != 1)
        dwHighScore = 0;

    fclose(fp);
    return dwHighScore;
}

/************************************************************
 *  @brief      Save high score
 *  @param[in]  dwHighScore     High score
 *  @retval     Nothing
 ************************************************************/
static void StSaveHighScore(unsigned int dwHighScore)
{
    FILE* fp = fopen(StGetHighScorePath(), "w");
    if (!fp)
        return;

    fprintf(fp, "%u\n", dwHighScore);
    fclose(fp);
}

/************************************************************
 *  @brief      Main
 *  @retval     0
 ************************************************************/
int main()
{
    // Game
    CStPlay play;
    play.NewGame((unsigned int)time(NULL));

    // The high score is stored by the platform, the game only holds it
    play.GetHighScore()->Set(StLoadHighScore());

    // View
    CStConsoleView view;

    // Enter screen
    StEnterRawMode();
    view.EnterScreen();

    bool bQuit = false;
    bool bSaved = false;
    long long llLastTick = StGetTick();

    view.Render(play);

    while (!bQuit)
    {
        switch (StGetKey())
        {
        case ST_KEY_QUIT:
            bQuit = true;
            break;

        case ST_KEY_RESTART:
            play.NewGame((unsigned int)time(NULL));
            bSaved = false;
            llLastTick = StGetTick();
            break;

        case ST_KEY_PAUSE:
            play.TogglePause();
            break;

        case ST_KEY_LEFT:
            play.MoveLeft();
            break;

        case ST_KEY_RIGHT:
            play.MoveRight();
            break;

        case ST_KEY_UP:
            play.Rotate();
            break;

        case ST_KEY_DOWN:
            play.SoftDrop();
            break;

        case ST_KEY_DROP:
            play.HardDrop();
            break;

        default:
            break;
        }

        // Hand the elapsed time to the game, it decides when to fall
        long long llNow = StGetTick();
        play.Tick((unsigned int)(llNow - llLastTick));
        llLastTick = llNow;

        // Persist the high score as soon as the game ends
        if (play.IsGameOver() && !bSaved)
        {
            StSaveHighScore(play.GetHighScore()->Get());
            bSaved = true;
        }

        view.Render(play);

        StSleep(ST_POLL_INTERVAL);
    }

    // Save high score
    StSaveHighScore(play.GetHighScore()->Get());

    // Leave screen
    view.LeaveScreen();
    StLeaveRawMode();

    printf("SCORE %u   HIGH SCORE %u   LEVEL %d   LINES %d\n",
        play.GetScore()->Get(), play.GetHighScore()->Get(),
        play.GetLevel(), play.GetLines());

    return 0;
}
