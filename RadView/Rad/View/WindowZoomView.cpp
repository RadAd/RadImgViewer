#include "WindowZoomView.h"

namespace
{
    inline SIZE operator*(double f, SIZE s)
    {
        s.cx = (LONG) (s.cx * f + 0.5);
        s.cy = (LONG) (s.cy * f + 0.5);
        return s;
    }
}

namespace rad
{
    WindowZoomView::WindowZoomView()
        : m_Zoom(200)
    {
    }

    SIZE WindowZoomView::GetViewSize() const { return m_Zoom / 100 * WindowScrollView::GetViewSize(); }

    void WindowZoomView::SetZoom(double Zoom)
    {
        double HorzScroll = GetScrollPercent(SB_HORZ);
        double VertScroll = GetScrollPercent(SB_VERT);

        m_Zoom = Zoom;
        UpdateScrollBars();

        SetScrollPercent(SB_HORZ, HorzScroll);
        SetScrollPercent(SB_VERT, VertScroll);

        UpdateScrollBars();
        InvalidateRect();
    }

    double WindowZoomView::GetScrollPercent(int nBar) const
    {
        SCROLLINFO	ScrollInfo;
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        GetScrollInfo(nBar, &ScrollInfo);

        if ((ScrollInfo.nMax - ScrollInfo.nMin) <= 0)
            return 0.5;
        else
        {
            double	Pos = ScrollInfo.nPos;
            if (ScrollInfo.nPage > 0)
                Pos += ScrollInfo.nPage / 2.0;
            return Pos / (double) (ScrollInfo.nMax - ScrollInfo.nMin);
        }
    }

    void WindowZoomView::SetScrollPercent(int nBar, double Percent)
    {
        SCROLLINFO	ScrollInfo;
        ScrollInfo.cbSize = sizeof(SCROLLINFO);
        ScrollInfo.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        GetScrollInfo(nBar, &ScrollInfo);

        if ((ScrollInfo.nMax - ScrollInfo.nMin) <= 0)
            ScrollInfo.nPos = 0;
        else
        {
            double	Pos = Percent * (ScrollInfo.nMax - ScrollInfo.nMin);
            if (ScrollInfo.nPage > 0)
                Pos -= ScrollInfo.nPage / 2.0;
            ScrollInfo.nPos = (int) Pos;
        }
        SetScrollInfo(nBar, &ScrollInfo, true);
    }
}
