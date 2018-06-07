#include "StatusWndChain.h"

#include <Rad\GUI\Window.h>

namespace rad
{
    LRESULT StatusWndChain::OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message)
        {
        case WM_SIZE:
            if (m_Status.IsAttached())
                m_Status.SendMessage(Message, wParam, lParam);
            break;

        case WM_MENUSELECT:
            if (m_Status.IsAttached())
            {
                HINSTANCE hInstance = (HINSTANCE) Window->GetWindowLongPtr(GWLP_HINSTANCE);
                TCHAR	Buffer[1024];
                LoadString(hInstance, LOWORD(wParam), Buffer, ARRAYSIZE(Buffer));
                m_Status.SetText(Buffer, SBT_NOBORDERS);
            }
            break;

        //case WM_CREATE:
            //m_Status.SetText(TEXT(""), SBT_NOBORDERS);
            //break;
        }

        return WindowChain::OnMessage(Window, Message, wParam, lParam);
    }
}
