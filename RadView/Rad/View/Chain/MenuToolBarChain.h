#pragma once

#include <Rad\GUI\WindowChain.h>

#include <Rad\GUI\Menu.h>
#include <Rad\GUI\CommCtrl.h>
#include <Rad\GUI\GdiObject.h>

#include <vector>

namespace rad
{
    class MenuDrawChain : public WindowChain
    {
    public:
        void SetMenu(MenuRef Menu, bool Popup = false, bool SetOwnerDraw = true)
        {
            m_Menu = Menu;
            if (SetOwnerDraw)
                SetMenuOwnerDraw(Menu, Popup);
        }

        void ClearMenu()
        {
            m_Menu.Release();
        }

        void SetImageList(ImageListRef hImageList)
        {
            m_ImageList = hImageList;
        }

        void SetAccelerator(HACCEL Accel);

    protected:
        virtual LRESULT OnMessage(Window *Wnd, UINT Message, WPARAM wParam, LPARAM lParam) override;

    protected:
        LRESULT OnDrawItem(Window *Wnd, UINT ID, LPDRAWITEMSTRUCT DrawItem);
        LRESULT OnMeasureItem(Window *Wnd, UINT ID, LPMEASUREITEMSTRUCT MeasureItem);

        bool GetAccelText(UINT CommandID, TCHAR *Text, size_t size);

        virtual int GetItemImageIndex(LPDRAWITEMSTRUCT DrawItem) = 0;

    private:
        static void SetMenuOwnerDraw(MenuRef Menu, bool Popup = false);

        MenuRef		m_Menu;
        ImageListRef	m_ImageList;
        Bitmap		m_MenuCheck;
        Bitmap		m_MenuRadio;

        std::vector<ACCEL> m_AccelTable;
    };

    class MenuToolBarChain : public MenuDrawChain
    {
    public:
        void SetToolBarWnd(WindowProxy Toolbar)
        {
            m_ToolBar.Attach(Toolbar.GetHWND());
            SetImageList(m_ToolBar.GetImageList());
        }

    protected:
        virtual int GetItemImageIndex(LPDRAWITEMSTRUCT DrawItem)
        {
            return m_ToolBar.IsAttached() ? m_ToolBar.GetBitmapIndex(DrawItem->itemID) : -1;
        }

    private:
        ToolBarWnd	m_ToolBar;
    };
}
