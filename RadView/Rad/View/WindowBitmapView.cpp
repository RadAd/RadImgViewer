#include "WindowBitmapView.h"

#include <Rad\GUI\RegClass.h>
#include <Rad\Rect.h>

#include "Chain\ScrollKeyChain.h"

namespace rad
{
    LPCTSTR WindowBitmapView::GetWndClassName(HINSTANCE hInstance)
    {
        RegClass rc(hInstance, _T("BitmapView"), DefWndHandlerWindowProc);
        rc.SetBackgroundColorType(COLOR_APPWORKSPACE);
        return MAKEINTATOM(rc.Register());
    }

    WindowBitmapView::WindowBitmapView()
    {
        Add(ScrollKeyChain::GetInstance());
    }

    void WindowBitmapView::SetBitmap(HBITMAP Bitmap)
    {
        m_bitmap.Attach(Bitmap);
#if 0
        if (m_bitmap.IsValid())
            m_bitmap.GetObject(&m_BitmapInfo);
        else
            ZeroMemory(&m_BitmapInfo, sizeof(m_BitmapInfo));
#endif
        UpdateScrollBars();
        InvalidateRect();
    }

    void WindowBitmapView::SetPalette(HPALETTE Palette)
    {
        m_palette.Attach(Palette);
        InvalidateRect();
    }

    SIZE WindowBitmapView::GetDocSize() const
    {
        BITMAP bm = {};
        if (m_bitmap.IsValid())
            m_bitmap.GetObject(&bm);
        return { bm.bmWidth, bm.bmHeight };
    }

    LRESULT WindowBitmapView::OnPaint(PaintDC& DC)
    {
        if (m_bitmap.IsValid())
        {
            {
                TempSelectObject	SelectPalette(DC, m_palette);
                DC.RealizePalette();

                BITMAP m_BitmapInfo = {};
                m_bitmap.GetObject(&m_BitmapInfo);

                RECT	ClientRect;
                GetClientRect(&ClientRect);

                SIZE	DocSize = GetViewSize();

                int	XPos = ::GetWidth(ClientRect) - DocSize.cx;
                int	YPos = ::GetHeight(ClientRect) - DocSize.cy;

                if (XPos < 0)
                    XPos = -GetScrollPos(SB_HORZ);
                else
                    XPos = XPos / 2;

                if (YPos < 0)
                    YPos = -GetScrollPos(SB_VERT);
                else
                    YPos = YPos / 2;

                MemDC	MemDC(DC);
                TempSelectObject	aSelectBitmap(MemDC, m_bitmap);

                DC.StretchBlt(XPos, YPos, DocSize.cx, DocSize.cy,
                    MemDC, 0, 0, m_BitmapInfo.bmWidth, m_BitmapInfo.bmHeight, SRCCOPY);
            }
            DC.RealizePalette();
        }
        return 0;
    }
}
