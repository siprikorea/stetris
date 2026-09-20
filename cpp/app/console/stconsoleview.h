#ifndef __STCONSOLEVIEW_H__
#define __STCONSOLEVIEW_H__

#include "../../stetris/stplay.h"

//
// Renders a CStPlay to the terminal. The logic layer knows nothing about
// this class - the UI pulls the state it needs and draws it.
//
class CStConsoleView
{
public:
    // Constructor
    CStConsoleView();

    // Enter screen
    void EnterScreen();
    // Leave screen
    void LeaveScreen();

    // Render one frame
    void Render(CStPlay& play);

protected:
    // Draw board with the side panel
    void DrawBoard(CStPlay& play);
    // Draw one line of the side panel
    void DrawSide(CStPlay& play, int nLine);
    // Draw help
    void DrawHelp();

    // Put cell
    void PutCell(int nType);
    // Put string
    void PutString(const char* pszText);

    // Screen buffer
    char m_szScreen[16384];
    // Screen length
    int m_nScreenLen;
};

#endif
