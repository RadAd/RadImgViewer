#define NOMINMAX

#include "MenuToolBarChain.h"

#include <Rad\GUI\Window.h>
#include <Rad\GUI\DevContext.h>

#include <Rad\Rect.h>

#include <algorithm>

#define	ODM_SUBMENU_SPACE		15
#define	ODM_ICON_BUTTON_SPACE	5
#define	ODM_STRING_SPACE		5

namespace
{
    SIZE GetMenuCheckSize(rad::DevContextRef DC)
    {
        SIZE s;
        s.cx = GetSystemMetrics(SM_CXMENUCHECK) * GetDeviceCaps(DC.GetHandle(), LOGPIXELSX) / 96 + 1;
        s.cy = GetSystemMetrics(SM_CYMENUCHECK) * GetDeviceCaps(DC.GetHandle(), LOGPIXELSY) / 96 + 1;
        return s;
    }

    rad::Bitmap CreateMonoBitmap(SIZE s)
    {
        BITMAP bm = {};
        bm.bmWidth = s.cx;
        bm.bmHeight = s.cy;
        bm.bmWidthBytes = (bm.bmWidth / 8 + 1) / 2 * 2;
        bm.bmPlanes = 1;
        bm.bmBitsPixel = 1;

        rad::Bitmap bmp;
        bmp.Create(&bm);
        return bmp;
    }

    rad::Bitmap CreateFrameControlBitmap(rad::DevContextRef DC, SIZE s, UINT Type, UINT State)
    {
        rad::Bitmap bmp = CreateMonoBitmap(s);

        rad::MemDC TempDC(DC);
        rad::TempSelectObject aBitmap(TempDC, bmp);
        RECT MenuCheckRect = { 0, 0, s.cx, s.cy };
        TempDC.DrawFrameControl(&MenuCheckRect, Type, State);

        return bmp;
    }
}

namespace rad
{
    void MenuDrawChain::SetAccelerator(HACCEL Accel)
    {
        int	Size = CopyAcceleratorTable(Accel, NULL, 0);
        if (Size > 0)
        {
            m_AccelTable.resize(Size);
            CopyAcceleratorTable(Accel, m_AccelTable.data(), Size);
        }
        else
            m_AccelTable.clear();
    }

    void MenuDrawChain::SetMenuOwnerDraw(MenuRef Menu, bool Popup)
    {
        int Count = Menu.GetItemCount();
        for (int I = 0; I < Count; ++I)
        {
            UINT	ID = Menu.GetItemID(I);
            if (ID == (UINT) -1)
            {	// a submenu
                if (Popup)
                {
                    MENUITEMINFO MenuItemInfo = { sizeof(MENUITEMINFO) };
                    MenuItemInfo.fMask = MIIM_TYPE;

                    Menu.GetItemInfo(I, &MenuItemInfo, TRUE);
                    MenuItemInfo.fType |= MFT_OWNERDRAW;
                    Menu.SetItemInfo(I, &MenuItemInfo, TRUE);
                }

                SetMenuOwnerDraw(Menu.GetSubMenu(I), true);
            }
            else if (ID == 0)
            {	// a seperator
            }
            else
            {
                MENUITEMINFO MenuItemInfo = { sizeof(MENUITEMINFO) };
                MenuItemInfo.fMask = MIIM_TYPE;

                Menu.GetItemInfo(I, &MenuItemInfo, TRUE);
                MenuItemInfo.fType |= MFT_OWNERDRAW;
                //			MenuItemInfo.fType |= MFT_RADIOCHECK;
                Menu.SetItemInfo(I, &MenuItemInfo, TRUE);
            }
        }
    }

    LRESULT MenuDrawChain::OnMessage(Window *Wnd, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message)
        {
#if 0
        case WM_INITMENUPOPUP:
            {
                m_Menu.Attach((HMENU) wParam);
                return WindowChain::OnMessage(Wnd, Message, wParam, lParam);
            }
#endif
        case WM_DRAWITEM:       return OnDrawItem(Wnd, (UINT) wParam, (LPDRAWITEMSTRUCT) lParam);
        case WM_MEASUREITEM:    return OnMeasureItem(Wnd, (UINT) wParam, (LPMEASUREITEMSTRUCT) lParam);
        default:                return WindowChain::OnMessage(Wnd, Message, wParam, lParam);
        }
    }

    LRESULT MenuDrawChain::OnDrawItem(Window* Wnd, UINT /*ID*/, LPDRAWITEMSTRUCT DrawItem)
    {
        if (DrawItem->CtlType == ODT_MENU && m_Menu.IsValid())
        {
            MENUITEMINFO MenuItemInfo = { sizeof(MENUITEMINFO) };
            MenuItemInfo.fMask = MIIM_TYPE;
            m_Menu.GetItemInfo(DrawItem->itemID, &MenuItemInfo, false);

            DevContextRef DC(DrawItem->hDC);
            RECT			BgRect(DrawItem->rcItem);
            RECT			TextRect(DrawItem->rcItem);
            bool	Checked = (DrawItem->itemState & ODS_CHECKED) != 0;
            bool	Grayed = (DrawItem->itemState & ODS_GRAYED) != 0;
            bool	Selected = (DrawItem->itemState & ODS_SELECTED) != 0;

            // Select the colors
            Pen	Pen;
            Pen.Create(GetSysColor(COLOR_MENU));
            Brush	Brush;

            if (Selected)
            {
                Brush.Attach(GetSysColorBrush(COLOR_HIGHLIGHT));
                DC.SetBkColor(GetSysColor(COLOR_HIGHLIGHT));
            }
            else
            {
                Brush.Attach(GetSysColorBrush(COLOR_MENU));
                DC.SetBkColor(GetSysColor(COLOR_MENU));
            }

            TempSelectObject aSelectPen(DC, Pen);
            TempSelectObject aSelectBrush(DC, Brush);

            {	// erase the background
                DC.Rectangle(BgRect.left, BgRect.top, BgRect.right, BgRect.bottom);
            }

            {	// Draw the button
                //			CommandS *Command = GetCommand(DrawItem->itemID);
                //			int	ImageIndex = Command ? Command->m_Image : -1;
                int	ImageIndex = GetItemImageIndex(DrawItem);

                SIZE	IconSize = m_ImageList.IsValid() ? m_ImageList.GetIconSize() : GetMenuCheckSize(DC); // TODO Assuming IconSize is larger
                IconSize.cx += ODM_ICON_BUTTON_SPACE;	IconSize.cy += ODM_ICON_BUTTON_SPACE;
                //			SIZE	IconSize = m_ToolBar.GetButtonSize();

                RECT	ButtonRect;
                ButtonRect.top = BgRect.top + (GetHeight(BgRect) - IconSize.cy) / 2 + 1;
                ButtonRect.left = BgRect.left + 2;
                ButtonRect.bottom = ButtonRect.top + IconSize.cy;	ButtonRect.right = ButtonRect.left + IconSize.cx;

                TextRect.left = ButtonRect.right;

                if (ImageIndex >= 0)
                {	// Draw the icon
                    BgRect.left = ButtonRect.right;
                    //if (Checked)
                    //DC.DrawFrameControl(&ButtonRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED | DFCS_ADJUSTRECT);
                    //else if (Selected)
                    //DC.DrawFrameControl(&ButtonRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_ADJUSTRECT);
                    //else
                    //DC.DrawFrameControl(&ButtonRect, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_FLAT | DFCS_ADJUSTRECT);

                    {
                        Icon	Icon(m_ImageList.GetIcon(ImageIndex, ILD_NORMAL));

                        UINT	Flags = DST_ICON;
                        if (Grayed)
                            Flags |= DSS_DISABLED;

                        // TODO Move into Icon
                        DrawState(DC.GetHandle(), GetSysColorBrush(COLOR_MENU), NULL, (LPARAM) Icon.GetHandle(), (WPARAM) 0,
                            ButtonRect.left, ButtonRect.top, GetWidth(ButtonRect), GetHeight(ButtonRect),
                            Flags);
                    }
                }
                else if (Checked)
                {	// Draw the checked image
                    BgRect.left = ButtonRect.right;

                    if (!m_MenuCheck.IsValid() || !m_MenuRadio.IsValid())
                    {
                        SIZE s = GetMenuCheckSize(DC);

                        if (!m_MenuCheck.IsValid())
                            m_MenuCheck = CreateFrameControlBitmap(DC, s, DFC_MENU, DFCS_MENUCHECK);

                        if (!m_MenuRadio.IsValid())
                            m_MenuRadio = CreateFrameControlBitmap(DC, s, DFC_MENU, DFCS_MENUBULLET);
                    }

                    HBITMAP	Bitmap = (MenuItemInfo.fType & MFT_RADIOCHECK) ? (HBITMAP) m_MenuRadio.GetHandle() : (HBITMAP) m_MenuCheck.GetHandle();

                    BITMAP	bmp;
                    m_MenuCheck.GetObject(&bmp);

                    UINT	Flags = DST_BITMAP;
                    if (Grayed)
                        Flags |= DSS_DISABLED;
                    DrawState(DC.GetHandle(), GetSysColorBrush(COLOR_MENU), NULL, (LPARAM) Bitmap, (WPARAM) 0,
                        ButtonRect.left + (GetWidth(ButtonRect) - bmp.bmWidth - 1) / 2, ButtonRect.top + (GetHeight(ButtonRect) - bmp.bmHeight - 1) / 2, bmp.bmWidth, bmp.bmHeight,
                        Flags);
                }
            }

            {	// Draw the string
                TCHAR	String[1024];
                int	Length = m_Menu.GetItemString(DrawItem->itemID, String, ARRAYSIZE(String), false);

                SIZE	StringSize;
                GetTextExtentPoint32(DC.GetHandle(), String, Length, &StringSize);
                TextRect.left += 3;
                TextRect.top += (GetHeight(TextRect) - StringSize.cy) / 2;

                UINT	Flags = DST_PREFIXTEXT;

                if (Selected & Grayed)
                {
                    DC.SetTextColor(GetSysColor(COLOR_3DHIGHLIGHT));
                }
                else if (Selected)
                {
                    DC.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
                }
                else if (Grayed)
                {
                    Flags |= DSS_DISABLED;
                }

                DrawState(DC.GetHandle(), (HBRUSH) Brush.GetHandle(), NULL, (LPARAM) String, (WPARAM) Length,
                    TextRect.left, TextRect.top, GetWidth(TextRect), GetHeight(TextRect),
                    Flags);

                if (GetAccelText(DrawItem->itemID, String, ARRAYSIZE(String)))
                {
                    //				Flags |= DSS_RIGHT;
                    SIZE	AccelStringSize;
                    Length = (int) _tcslen(String);
                    GetTextExtentPoint32(DC.GetHandle(), String, Length, &AccelStringSize);
                    TextRect.left = TextRect.right - AccelStringSize.cx - ODM_SUBMENU_SPACE;

                    DrawState(DC.GetHandle(), (HBRUSH) Brush.GetHandle(), NULL, (LPARAM) String, (WPARAM) Length,
                        TextRect.left, TextRect.top, GetWidth(TextRect), GetHeight(TextRect),
                        Flags);
                }
            }

            return TRUE;
        }
        else
            return DoDefault(Wnd);
    }

    LRESULT MenuDrawChain::OnMeasureItem(Window* Wnd, UINT /*ID*/, LPMEASUREITEMSTRUCT MeasureItem)
    {
        if (MeasureItem->CtlType == ODT_MENU && m_Menu.IsValid())
        {
            MENUITEMINFO MenuItemInfo = { sizeof(MENUITEMINFO) };
            MenuItemInfo.fMask = MIIM_TYPE | MIIM_SUBMENU | MIIM_ID;
            m_Menu.GetItemInfo(MeasureItem->itemID, &MenuItemInfo, false);

            WindowDC	DC(*Wnd);

            NONCLIENTMETRICS nm;
            nm.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &nm, 0);

            Font	Font;
            Font.Create(&nm.lfMenuFont);
            TempSelectObject aSelectFont(DC, Font);

            SIZE	IconSize = m_ImageList.IsValid() ? m_ImageList.GetIconSize() : GetMenuCheckSize(DC); // TODO Assuming IconSize is larger
            IconSize.cx += ODM_ICON_BUTTON_SPACE;	IconSize.cy += ODM_ICON_BUTTON_SPACE;
            //		SIZE	IconSize = m_ToolBar.GetButtonSize();

            TCHAR	String[1024];
            int	Length = m_Menu.GetItemString(MeasureItem->itemID, String, ARRAYSIZE(String), false);

            SIZE	StringSize;
            GetTextExtentPoint32(DC.GetHandle(), String, Length, &StringSize);

            if (GetAccelText(MeasureItem->itemID, String, ARRAYSIZE(String)))
            {
                SIZE	AccelStringSize;
                GetTextExtentPoint32(DC.GetHandle(), String, (int) _tcslen(String), &AccelStringSize);
                StringSize.cx += AccelStringSize.cx + ODM_STRING_SPACE;
            }

            MeasureItem->itemWidth = IconSize.cy + StringSize.cx;
            MeasureItem->itemHeight = std::max((int) IconSize.cx, (int) StringSize.cy);
            if (MenuItemInfo.hSubMenu != NULL)
                MeasureItem->itemWidth += ODM_SUBMENU_SPACE;
            return TRUE;
        }
        else
            return DoDefault(Wnd);
    }

    bool MenuDrawChain::GetAccelText(UINT CommandID, TCHAR *Text, size_t size)
    {
        bool Found = false;

        Text[0] = '\0';

        for (const ACCEL& AccelTable : m_AccelTable)
        {
            if (AccelTable.cmd == CommandID)
            {
                Found = true;

                if (AccelTable.fVirt & FCONTROL)
                    _tcscat_s(Text, size, TEXT("Ctrl+"));
                if (AccelTable.fVirt & FALT)
                    _tcscat_s(Text, size, TEXT("Alt+"));
                if (AccelTable.fVirt & FSHIFT)
                    _tcscat_s(Text, size, TEXT("Shift+"));

                if (AccelTable.fVirt & FVIRTKEY)
                {
                    if ((AccelTable.key >= '0' && AccelTable.key <= '9')
                        || (AccelTable.key >= 'A' && AccelTable.key <= 'Z'))
                    {
                        TCHAR str[] = { (TCHAR) AccelTable.key, TEXT('\0') };
                        _tcscat_s(Text, size, str);
                    }
                    else
                    {
                        switch (AccelTable.key)
                        {
                        case VK_ESCAPE:     _tcscat_s(Text, size, TEXT("Esc")); break;
                        case VK_ADD:        _tcscat_s(Text, size, TEXT("+"));   break;
                        case VK_SUBTRACT:   _tcscat_s(Text, size, TEXT("-"));   break;
                        case VK_MULTIPLY:   _tcscat_s(Text, size, TEXT("*"));   break;
                        default:            _tcscat_s(Text, size, TEXT("Virt?"));   break;
                        }
                    }
                }
                else
                {
                    TCHAR str[] = { (TCHAR) AccelTable.key, TEXT('\0') };
                    _tcscat_s(Text, size, str);
                }

                break;
            }
        }

        return Found;
    }
}
