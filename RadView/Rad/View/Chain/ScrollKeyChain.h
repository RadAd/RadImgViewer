#pragma once

#include <Rad\GUI\WindowChain.h>

#include <Rad\GUI\Window.h>

namespace rad
{
    class ScrollKeyChain : public WindowChain
    {
    private:
        ScrollKeyChain()
        {
        }

    public:
        typedef Window::KeyInfoT KeyInfoT;

        static ScrollKeyChain* GetInstance()
        {
            static ScrollKeyChain skl;
            return &skl;
        }

    protected:
        virtual LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override;

    protected:
        LRESULT OnKeyDown(Window* Wnd, int VirtKey, const KeyInfoT *KeyInfo);
        LRESULT OnMouseWheel(Window* Wnd, int KeyFlags, short Distance, const POINTS &Point);
        LRESULT OnMiddleButtonDown(Window* Wnd, int KeyFlags, const POINTS &Point);
        LRESULT OnMiddleButtonUp(Window* Wnd, int KeyFlags, const POINTS &Point);
        LRESULT OnMouseMove(Window* Wnd, int KeyFlags, const POINTS &Point);

    private:
        HWND    m_hWnd = NULL;
        POINTS	m_OrigPoint;
        int		m_HorzTrackPoint;
        int		m_VertTrackPoint;
        HCURSOR	m_OrigCursor;
    };
}
