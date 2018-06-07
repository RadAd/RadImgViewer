#pragma once

#include "WindowZoomView.h"

#include <Rad\GUI\GdiObject.h>

namespace rad
{
    class WindowBitmapView : public WindowZoomView
    {
    public:
        WindowBitmapView();

        LPCTSTR GetWndClassName(HINSTANCE hInstance) override;

        void SetBitmap(HBITMAP Bitmap);
        void SetPalette(HPALETTE Palette);

    protected:
        SIZE GetDocSize() const override;

    protected:
        LRESULT OnPaint(PaintDC& DC) override;

    private:
        Bitmap m_bitmap;
        Palette m_palette;
    };
}
