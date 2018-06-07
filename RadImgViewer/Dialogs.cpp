#include "Dialogs.h"

#include <Rad\GUI\Controls.h>

#include "ImgViewerDoc.h"
#include "resource.h"

#include <FreeImage.H>

namespace
{
    inline bool DoChooseColor(HWND hWnd, COLORREF& color)
    {
        static COLORREF s_CustColors[16] = { 0 };

        CHOOSECOLOR Color = { sizeof(Color) };
        Color.hwndOwner = hWnd;
        Color.rgbResult = color;
        Color.Flags = CC_ANYCOLOR | CC_RGBINIT;
        Color.lpCustColors = s_CustColors;

        if (ChooseColor(&Color))
        {
            color = Color.rgbResult;
            return true;
        }
        else
            return false;
    }
}

// **** AboutDlg

INT_PTR AboutDlg::DoModal(HWND hParent, HINSTANCE hInstance)
{
    TCHAR	FileName[1024];
    GetModuleFileName(hInstance, FileName, 1024);

    DWORD	Dummy;
    DWORD	Size = GetFileVersionInfoSize(FileName, &Dummy);

    if (Size > 0)
    {
        void	*Info = malloc(Size);

        // VS_VERSION_INFO   VS_VERSIONINFO  VS_FIXEDFILEINFO

        GetFileVersionInfo(FileName, Dummy, Size, Info);

        TCHAR	*String;
        UINT	Length;
        VerQueryValue(Info, TEXT("\\StringFileInfo\\0c0904b0\\FileVersion"), (LPVOID *) &String, &Length);
        _tcscpy_s(m_Version, String);
        VerQueryValue(Info, TEXT("\\StringFileInfo\\0c0904b0\\ProductName"), (LPVOID *) &String, &Length);
        _tcscpy_s(m_Product, String);

        free(Info);
    }
    else
    {
        _tcscpy_s(m_Version, TEXT("Unknown"));
    }

    return Dialog::DoModal(hInstance, IDD_ABOUT, hParent);
}

BOOL AboutDlg::OnInitDialog(HWND FocusControl)
{
    rad::WindowProxy	AppIcon(GetDlgItem(IDC_ABOUT_APPICON));
    rad::WindowProxy	Product(GetDlgItem(IDC_ABOUT_PRODUCT));
    rad::WindowProxy	Version(GetDlgItem(IDC_ABOUT_VERSION));

    if (m_Icon)
        AppIcon.SendMessage(STM_SETICON, (WPARAM) m_Icon.GetHandle());
    Product.SetWindowText(m_Product);
    Version.SetWindowText(m_Version);

    return Dialog::OnInitDialog(FocusControl);
}

BOOL AboutDlg::OnCommand(WORD NotifyCode, WORD ID, HWND hWnd)
{
    if (NotifyCode == BN_CLICKED)
    {
        switch (ID)
        {
        case IDC_ABOUT_WEBSITE:
        case IDC_ABOUT_MAIL:
            {
                TCHAR	Url[1024];
                rad::WindowProxy	Button(hWnd);
                Button.GetWindowText(Url, 1024);
                //WinExec(Url, SW_SHOW);
                ShellExecute(GetHWND(), TEXT("open"), Url, NULL, NULL, SW_SHOW);
            }
            break;
        }
    }

    return Dialog::OnCommand(NotifyCode, ID, hWnd);
}

// **** ImgViewerAboutDlg

ImgViewerAboutDlg::ImgViewerAboutDlg(HINSTANCE hInstance)
    : AboutDlg(::LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMGVIEWER)))
{
}

INT_PTR ImgViewerAboutDlg::DoModal(HWND hParent, HINSTANCE hInst)
{
    return AboutDlg::DoModal(hParent, hInst);
}

BOOL ImgViewerAboutDlg::OnInitDialog(HWND FocusControl)
{
    SetDlgItemTextA(IDC_ABOUT_FREEIMAGE_VERSION, FreeImage_GetVersion());
    SetDlgItemTextA(IDC_ABOUT_FREEIMAGE_LICENSE, FreeImage_GetCopyrightMessage());
    return AboutDlg::OnInitDialog(FocusControl);
}

// **** ColourQuantizeDlg

INT_PTR ColourQuantizeDlg::DoModal(HWND hParent, HINSTANCE hInst)
{
    return Dialog::DoModal(hInst, IDD_QUANTIZE, hParent);
}

BOOL ColourQuantizeDlg::OnInitDialog(HWND FocusControl)
{
    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    const option* o = Quantize;
    while (o->name)
    {
        int i = Method.AddString(o->name);
        Method.SetItemData(i, o->value);
        ++o;
    }
    Method.SetCurSel(0);
    return Dialog::OnInitDialog(FocusControl);
}

BOOL ColourQuantizeDlg::OnOk()
{
    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    int i = Method.GetCurSel();
    m_Method = Quantize[i].value;
    return Dialog::OnOk();
}

// **** DitherDlg

INT_PTR DitherDlg::DoModal(HWND hParent, HINSTANCE hInst)
{
    return Dialog::DoModal(hInst, IDD_DITHER, hParent);
}

BOOL DitherDlg::OnInitDialog(HWND FocusControl)
{
    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    const option* o = Dither;
    while (o->name)
    {
        int i = Method.AddString(o->name);
        Method.SetItemData(i, o->value);
        ++o;
    }
    Method.SetCurSel(0);
    return Dialog::OnInitDialog(FocusControl);
}

BOOL DitherDlg::OnOk()
{
    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    int i = Method.GetCurSel();
    m_Method = Dither[i].value;
    return Dialog::OnOk();
}

// **** CRescaleDlg

INT_PTR RescaleDlg::DoModal(HWND hParent, HINSTANCE hInst)
{
    return Dialog::DoModal(hInst, IDD_RESCALE, hParent);
}

BOOL RescaleDlg::OnInitDialog(HWND FocusControl)
{
    SetDlgItemInt(IDC_WIDTH, m_Width, false);
    SetDlgItemInt(IDC_HEIGHT, m_Height, false);

    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    const option* o = Rescale;
    while (o->name)
    {
        int i = Method.AddString(o->name);
        Method.SetItemData(i, o->value);
        ++o;
    }
    Method.SetCurSel(0);
    return Dialog::OnInitDialog(FocusControl);
}

BOOL RescaleDlg::OnOk()
{
    m_Width = GetDlgItemInt(IDC_WIDTH, false);
    m_Height = GetDlgItemInt(IDC_HEIGHT, false);
    rad::ComboBoxWnd Method(GetDlgItem(IDC_METHOD));
    int i = Method.GetCurSel();
    m_Method = Rescale[i].value;
    return Dialog::OnOk();
}

// **** CDoubleDlg

BOOL DoubleDlg::OnOk()
{
    TCHAR text[1024];
    GetDlgItemText(m_Item, text, 1024);
    m_Double = _ttof(text);
    return Dialog::OnOk();
}

// **** CRecolourDlg

INT_PTR RecolourDlg::DoModal(HWND hParent, HINSTANCE hInst)
{
    return Dialog::DoModal(hInst, IDD_RECOLOUR, hParent);
}

BOOL RecolourDlg::OnInitDialog(HWND FocusControl)
{
    TCHAR text[1024];
    _stprintf_s(text, 1024, _T("%f"), m_Tolerance);
    SetDlgItemText(IDC_TOLERANCE, text);
    return Dialog::OnInitDialog(FocusControl);
}

BOOL RecolourDlg::OnCommand(WORD NotifyCode, WORD ID, HWND hWnd)
{
    switch (ID)
    {
    case IDC_SOURCE_COLOUR:
        if (DoChooseColor(GetHWND(), m_SourceColor))
        {
            m_SourceBrush.Create(m_SourceColor);
            ::InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case IDC_DEST_COLOUR:
        if (DoChooseColor(GetHWND(), m_DestColor))
        {
            m_DestBrush.Create(m_DestColor);
            ::InvalidateRect(hWnd, NULL, TRUE);
        }
        break;
    }
    return Dialog::OnCommand(NotifyCode, ID, hWnd);
}

BOOL RecolourDlg::OnMessage(UINT Message, WPARAM wParam, LPARAM lParam)
{
    switch (Message)
    {
    case WM_CTLCOLORBTN:
        //HDC hdc = (HDC) wParam;
        HWND hwnd = (HWND) lParam;
        switch (GetDlgCtrlID(hwnd))
        {
        case IDC_SOURCE_COLOUR:
            return (BOOL) (INT_PTR) m_SourceBrush.GetHandle();

        case IDC_DEST_COLOUR:
            return (BOOL) (INT_PTR) m_DestBrush.GetHandle();
        }
        break;
    }
    return Dialog::OnMessage(Message, wParam, lParam);
}

BOOL RecolourDlg::OnOk()
{
    TCHAR text[1024];
    GetDlgItemText(IDC_TOLERANCE, text, 1024);
    m_Tolerance = _ttof(text);
    return Dialog::OnOk();
}
