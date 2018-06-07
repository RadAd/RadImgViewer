#include "ScrollKeyChain.h"

namespace rad
{
    LRESULT ScrollKeyChain::OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message)
        {
        case WM_KEYDOWN:        return OnKeyDown(Window, (int) wParam, (KeyInfoT*) &lParam);
        //case WM_KEYUP:        return OnKeyUp(Window, (int) wParam, (KeyInfoT*) &lParam);
        case WM_MOUSEWHEEL:     return OnMouseWheel(Window, (int) LOWORD(wParam), (short) HIWORD(wParam), MAKEPOINTS(lParam));
        case WM_MBUTTONDOWN:    return OnMiddleButtonDown(Window, (UINT) wParam, MAKEPOINTS(lParam));
        case WM_MBUTTONUP:      return OnMiddleButtonUp(Window, (UINT) wParam, MAKEPOINTS(lParam));
        case WM_MOUSEMOVE:      return OnMouseMove(Window, (UINT) wParam, MAKEPOINTS(lParam));
        default:                return WindowChain::OnMessage(Window, Message, wParam, lParam);
        }
    }

    inline LRESULT ScrollKeyChain::OnKeyDown(Window* Wnd, int VirtKey, const KeyInfoT* /*KeyInfo*/)
    {
        switch (VirtKey)
        {
        case VK_UP:
            Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP, Wnd->GetScrollPos(SB_VERT)), 0);
            break;

        case VK_DOWN:
            Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN, Wnd->GetScrollPos(SB_VERT)), 0);
            break;

        case VK_LEFT:
            Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT, Wnd->GetScrollPos(SB_HORZ)), 0);
            break;

        case VK_RIGHT:
            Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT, Wnd->GetScrollPos(SB_HORZ)), 0);
            break;

        case VK_PRIOR:
            if (HIWORD(GetKeyState(VK_SHIFT)))
                Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_PAGELEFT, Wnd->GetScrollPos(SB_HORZ)), 0);
            else
                Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEUP, Wnd->GetScrollPos(SB_VERT)), 0);
            break;

        case VK_NEXT:
            if (HIWORD(GetKeyState(VK_SHIFT)))
                Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_PAGERIGHT, Wnd->GetScrollPos(SB_HORZ)), 0);
            else
                Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_PAGEDOWN, Wnd->GetScrollPos(SB_VERT)), 0);
            break;

        case VK_HOME:
            if (HIWORD(GetKeyState(VK_SHIFT)))
                Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_LEFT, Wnd->GetScrollPos(SB_HORZ)), 0);
            else
                Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_TOP, Wnd->GetScrollPos(SB_VERT)), 0);
            break;

        case VK_END:
            if (HIWORD(GetKeyState(VK_SHIFT)))
                Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_RIGHT, Wnd->GetScrollPos(SB_HORZ)), 0);
            else
                Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_BOTTOM, Wnd->GetScrollPos(SB_VERT)), 0);
            break;
        }
        //return WindowChain::OnKeyDown(Wnd, VirtKey, KeyInfo);
        return DoDefault(Wnd);
    }

    LRESULT ScrollKeyChain::OnMouseWheel(Window* Wnd, int KeyFlags, short Distance, const POINTS& /*Point*/)
    {
        if (KeyFlags & MK_CONTROL)
        {
            if (KeyFlags & MK_SHIFT)
            {
                if (Distance > 0)
                    Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_LINELEFT, Wnd->GetScrollPos(SB_HORZ)), 0);
                else if (Distance < 0)
                    Wnd->SendMessage(WM_HSCROLL, MAKELONG(SB_LINERIGHT, Wnd->GetScrollPos(SB_HORZ)), 0);
            }
            else
            {
                if (Distance > 0)
                    Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEUP, Wnd->GetScrollPos(SB_VERT)), 0);
                else if (Distance < 0)
                    Wnd->SendMessage(WM_VSCROLL, MAKELONG(SB_LINEDOWN, Wnd->GetScrollPos(SB_VERT)), 0);
            }
        }
        //return WindowChain::OnMouseWheel(Wnd, KeyFlags, Distance, Point);
        return DoDefault(Wnd);
    }

    LRESULT ScrollKeyChain::OnMiddleButtonDown(Window* Wnd, int /*KeyFlags*/, const POINTS &Point)
    {
        Wnd->SetCapture();

        assert(m_hWnd == NULL);
        m_hWnd = Wnd->GetHWND();
        m_OrigPoint = Point;

        SCROLLINFO	si = { sizeof(SCROLLINFO) };
        si.fMask = SIF_ALL;
        Wnd->GetScrollInfo(SB_HORZ, &si);
        m_HorzTrackPoint = si.nPos;
        Wnd->GetScrollInfo(SB_VERT, &si);
        m_VertTrackPoint = si.nPos;

        m_OrigCursor = SetCursor(LoadCursor(NULL, IDC_SIZEALL));

        //return WindowChain::OnMiddleButtonDown(Wnd, KeyFlags, Point);
        return DoDefault(Wnd);
    }

    LRESULT ScrollKeyChain::OnMiddleButtonUp(Window* Wnd, int /*KeyFlags*/, const POINTS& /*Point*/)
    {
        if (m_hWnd == Wnd->GetHWND())
        {
            SetCursor(m_OrigCursor);
            Wnd->ReleaseCapture();
            m_hWnd = NULL;
        }
        //return WindowChain::OnMiddleButtonUp(Wnd, KeyFlags, Point);
        return DoDefault(Wnd);
    }

    LRESULT ScrollKeyChain::OnMouseMove(Window* Wnd, int /*KeyFlags*/, const POINTS &Point)
    {
        if (m_hWnd == Wnd->GetHWND())
        {
            SCROLLINFO	si = { sizeof(SCROLLINFO) };
            si.fMask = SIF_ALL;

            Wnd->GetScrollInfo(SB_HORZ, &si);
            si.nPos = m_HorzTrackPoint + (m_OrigPoint.x - Point.x);
            Wnd->SetScrollInfo(SB_HORZ, &si, true);

            Wnd->GetScrollInfo(SB_VERT, &si);
            si.nPos = m_VertTrackPoint + (m_OrigPoint.y - Point.y);
            Wnd->SetScrollInfo(SB_VERT, &si, true);

            Wnd->InvalidateRect();
        }
        //return WindowChain::OnMouseMove(Wnd, KeyFlags, Point);
        return DoDefault(Wnd);
    }
}
