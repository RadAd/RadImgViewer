#include "WindowScrollView.h"

#include <Rad\GUI\GdiObject.h>
#include <Rad\Rect.h>

namespace rad
{
    WindowScrollView::WindowScrollView(bool TrackUpdate)
        : m_TrackUpdate(TrackUpdate)
        , m_UpdatingScrollBars(false)
    {}

    void WindowScrollView::UpdateScrollBars()
    {
        if (!m_UpdatingScrollBars)
        {
            m_UpdatingScrollBars = true;

            RECT	ClientRect;
            GetClientRect(&ClientRect);

            SIZE	DocSize = GetViewSize();

            SCROLLINFO	ScrollInfo = { sizeof(SCROLLINFO) };
            ScrollInfo.fMask = SIF_RANGE | SIF_PAGE; // | SIF_DISABLENOSCROLL;

            ScrollInfo.nMin = 0;

            ScrollInfo.nMax = DocSize.cy;
            ScrollInfo.nPage = ::GetHeight(ClientRect);
            SetScrollInfo(SB_VERT, &ScrollInfo, true);

            ScrollInfo.nMax = DocSize.cx;
            ScrollInfo.nPage = ::GetWidth(ClientRect);
            SetScrollInfo(SB_HORZ, &ScrollInfo, true);

            m_UpdatingScrollBars = false;
        }
    }

    LRESULT WindowScrollView::OnCreate(LPCREATESTRUCT CreateStruct)
    {
        // TODO Scroll bars not shown on initial update
        //CreateStruct->style |= WS_VSCROLL;
        //CreateStruct->style |= WS_HSCROLL;
        LRESULT r = Window::OnCreate(CreateStruct);
        //UpdateScrollBars();
        return r;
    }

    LRESULT WindowScrollView::OnEraseBackground(DevContextRef DC)
    {
        // erase around the document, assume it is centred
        RECT	ClientRect;
        GetClientRect(&ClientRect);

        ULONG_PTR bg = GetClassLongPtr(GCLP_HBRBACKGROUND);
        if (bg > 0)
        {
            BrushRef	Brush;
            //Brush.Attach(GetSysColorBrush(COLOR_APPWORKSPACE));
            Brush.Attach(bg < 31 ? (HBRUSH) GetSysColorBrush((int) (bg - 1)) : (HBRUSH) bg);

            SIZE	DocSize = GetViewSize();

            if (DocSize.cx == 0 || DocSize.cy == 0)
            {
                DC.FillRect(&ClientRect, Brush.GetBrushHandle());
            }
            else
            {
                if (DocSize.cx < ::GetWidth(ClientRect))
                {
                    int Space = (::GetWidth(ClientRect) - DocSize.cx) / 2;

                    RECT	LeftRect(ClientRect);
                    LeftRect.left += Space;
                    DC.FillRect(&LeftRect, Brush.GetBrushHandle());

                    RECT	RightRect(ClientRect);
                    RightRect.right -= Space;
                    DC.FillRect(&RightRect, Brush.GetBrushHandle());

                    ClientRect.left += Space;
                    ClientRect.right -= Space;
                }
                if (DocSize.cy < ::GetHeight(ClientRect))
                {
                    int Space = (::GetHeight(ClientRect) - DocSize.cy) / 2;

                    RECT	TopRect(ClientRect);
                    TopRect.top += Space;
                    DC.FillRect(&TopRect, Brush.GetBrushHandle());

                    RECT	BottomRect(ClientRect);
                    BottomRect.bottom -= Space;
                    DC.FillRect(&BottomRect, Brush.GetBrushHandle());
                }
            }
            return !0;
        }
        else
            return 0;
    }

    LRESULT WindowScrollView::OnHScroll(HWND ScrollBar, int Code, int Pos)
    {
        RECT	Rect;
        GetClientRect(&Rect);
        OnScroll(SB_HORZ, Code, GetHeight(Rect));

        return Window::OnHScroll(ScrollBar, Code, Pos);
    }

    LRESULT WindowScrollView::OnVScroll(HWND ScrollBar, int Code, int Pos)
    {
        RECT	Rect;
        GetClientRect(&Rect);
        OnScroll(SB_VERT, Code, GetWidth(Rect));

        return Window::OnVScroll(ScrollBar, Code, Pos);
    }

    LRESULT WindowScrollView::OnSize(UINT Type, int cx, int cy)
    {
        LRESULT r = Window::OnSize(Type, cx, cy);
        UpdateScrollBars();
        return r;
    }

    void WindowScrollView::OnScroll(int Bar, int Code, int DefaultPage)
    {
        bool	Update = false;

        SCROLLINFO si = {};
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_ALL;
        GetScrollInfo(Bar, &si);

        if (si.nPage <= 0)
            si.nPage = DefaultPage;

        switch (Code)
        {
            //	case SB_LINEUP:
        case SB_LINELEFT:
            {
                Update = true;
                double ddelta = (double) (si.nPage) * 0.10;
                int	delta = (int) (ddelta + 0.5);
                if (delta <= 0)
                    delta = 1;
                si.nPos -= delta;
            }
            break;

            //	case SB_LINEDOWN:
        case SB_LINERIGHT:
            {
                Update = true;
                double ddelta = (double) (si.nPage) * 0.10;
                int	delta = (int) (ddelta + 0.5);
                if (delta <= 0)
                    delta = 1;
                si.nPos += delta;
            }
            break;

            //	case SB_PAGEUP:
        case SB_PAGELEFT:
            Update = true;
            si.nPos -= si.nPage;
            break;

            //	case SB_PAGEDOWN:
        case SB_PAGERIGHT:
            Update = true;
            si.nPos += si.nPage;
            break;

        case SB_THUMBPOSITION:
            if (!m_TrackUpdate)
            {
                Update = true;
                si.nPos = si.nTrackPos;
            }
            break;

        case SB_THUMBTRACK:
            if (m_TrackUpdate)
            {
                Update = true;
                si.nPos = si.nTrackPos;
            }
            break;

            //	case SB_TOP:
        case SB_LEFT:
            Update = true;
            si.nPos = si.nMin;
            break;

            //	case SB_BOTTOM:
        case SB_RIGHT:
            Update = true;
            si.nPos = si.nMax;
            break;

        case SB_ENDSCROLL:
            break;
        }

        if (si.nPos < si.nMin)
            si.nPos = si.nMin;
        if (si.nPos > si.nMax)
            si.nPos = si.nMax;

        if (Update)
        {
            SetScrollInfo(Bar, &si, true);
            OnScrollBarUpdate(Bar);
        }
    }

    void WindowScrollView::OnScrollBarUpdate(int /*Bar*/)
    {
        InvalidateRect();
    }
}
