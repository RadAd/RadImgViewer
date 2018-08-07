#define NOMINMAX

#include <Rad\GUI\WindowCreate.h>
#include <Rad\GUI\MessageLoop.h>
#include <Rad\GUI\RegClass.h>
#include <Rad\Rect.h>

#include <Rad\View\FrameWindow.h>
#include <Rad\View\WindowBitmapView.h>

#include <FreeImage.H>

#include "ImgViewerDoc.h"
#include "Dialogs.h"
#include "FreeImageHelper.h"
#include "resource.h"

#define APP_NAME_A "ImgViewer"
#define APP_NAME _T(APP_NAME_A)

using namespace rad;

namespace
{
    HINSTANCE g_hInstance = NULL;
    HACCEL g_hAccel = NULL;

    const TCHAR SEP = '\0';
}

class ImgViewerView : public WindowBitmapView, public CImgViewerDocListener
{
public: // CImgViewerDocListener
    void ImgViewerDocMsg(CImgViewerDoc* pDoc, int Msg) override
    {
        if (Msg & IVDL_PALETTE_CHANGED)
        {
            SetPalette(pDoc->CreatePalette());
        }
        if (Msg & IVDL_BITS_CHANGED)
        {
            rad::WindowDC	DC(*this);
            SetBitmap(pDoc->CreateBitmap(DC));
        }
        if (Msg & IVDL_SIZE_CHANGED)
        {
            UpdateScrollBars();
            InvalidateRect();
        }
    }

    void ImgViewerDocError(CImgViewerDoc* /*pDoc*/, const std::string& Msg) override
    {
        MessageBoxA(GetHWND(), Msg.c_str(), APP_NAME_A, MB_OK | MB_ICONERROR);
    }

protected:
    LRESULT OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override
    {
        switch (ID)
        {
        case ID_VIEW_ZOOM_INCREASE:
            if (GetZoom() < 10000)
                SetZoom(GetZoom() * 2);
            break;

        case ID_VIEW_ZOOM_ORIGINAL:
            SetZoom(100);
            break;

        case ID_VIEW_ZOOM_DECREASE:
            if (GetZoom() > 1)
                SetZoom(GetZoom() / 2);
            break;

        case ID_VIEW_ZOOM_TOFIT:
            {
                SIZE docSize = GetDocSize();
                RECT rect;
                GetClientRect(&rect);
                SIZE zoom;
                zoom.cx = docSize.cx > 0 ? GetWidth(rect) * 100 / docSize.cx : 0;
                zoom.cy = docSize.cy > 0 ? GetHeight(rect) * 100 / docSize.cy : 0;
                //SetZoom(std::min(zoom.cx, zoom.cy));
                SetZoom(zoom.cx < zoom.cy ? zoom.cx : zoom.cy);
            }
            break;

        default:
            return WindowBitmapView::OnCommand(NotifyCode, ID, hWnd);
        }
        return 0;
    }

    LRESULT OnMouseWheel(int Flags, short Distance, POINTS Point) override
    {
        if ((Flags & MK_CONTROL) == 0)
        {
            if (Distance > 0)
                SendMessage(WM_COMMAND, ID_VIEW_ZOOM_INCREASE);
            else if (Distance < 0)
                SendMessage(WM_COMMAND, ID_VIEW_ZOOM_DECREASE);
            // Do not pass on to parent (which Default() does)
            return 0;
        }
        else
            return WindowBitmapView::OnMouseWheel(Flags, Distance, Point);
    }
};

class ImgViewerFrame : public FrameWindow, public CommandStatus, public CImgViewerDocListener
{
protected:
    typedef FrameWindow Base;

public:
    virtual WindowCreate GetWindowCreate(HINSTANCE hInstance) override
    {
        WindowCreate wc = Window::GetWindowCreate(hInstance);
        wc.hMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU));
        return wc;
    }

    ImgViewerFrame()
    {
        m_Doc.AddListener(this);

        ImgViewerView* View = new ImgViewerView;
        m_Doc.AddListener(View);
        SetView(View);
    }

    void Load(LPCTSTR FileName)
    {
        if (FileName)
        {
            HCURSOR OldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
            if (GetStatusWnd().IsAttached())
                GetStatusWnd().SetText(_T("Loading..."), SBT_NOBORDERS);
            m_Doc.Load(FileName);
            SendMessage(WM_COMMAND, ID_VIEW_ZOOM_TOFIT);
            SetCursor(OldCursor);
        }
        else
        {
            m_Doc.Close();
        }

        if (GetStatusWnd().IsAttached())
            GetStatusWnd().SetText(_T(""), SBT_NOBORDERS);
    }

protected:
    virtual void CreateChildren(LPCREATESTRUCT CreateStruct) override
    {
        GetToolBarWnd().Create(*this, WS_VISIBLE | WS_CHILD | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE, 1010, CreateStruct->hInstance, IDR_TOOLBAR);
        GetStatusWnd().Create(*this, WS_VISIBLE | WS_CHILD | SBARS_SIZEGRIP, 100, NULL);

        FrameWindow::CreateChildren(CreateStruct);
    }

protected:
    LPCTSTR GetWndClassName(HINSTANCE hInstance) override
    {
        RegClass rc(hInstance, _T("ImgViewer"), DefWndHandlerWindowProc);
        rc.SetIcon(IDI_IMGVIEWER);
        rc.SetBackgroundColorType(COLOR_APPWORKSPACE);
        return MAKEINTATOM(rc.Register());
    }

    virtual LRESULT OnCreate(LPCREATESTRUCT CreateStruct) override
    {
        LRESULT res = Base::OnCreate(CreateStruct);

        SetAccelerator(g_hAccel);
        DragAcceptFiles();
        AddClipboardFormatListener(GetHWND());

        return res;
    }

    virtual LRESULT OnSize(UINT Type, int cx, int cy) override
    {
        Base::OnSize(Type, cx, cy);

        // TODO Move into frame window or maybe status chain
        if (GetStatusWnd().IsAttached())
        {
            int StatusSize[] = { 100, 100 ,100 };
            int StatusParts[ARRAYSIZE(StatusSize) + 1];
            StatusParts[ARRAYSIZE(StatusParts) - 1] = cx - 5;
            if (GetStatusWnd().GetWindowLongPtr(GWL_STYLE) & SBARS_SIZEGRIP)
                StatusParts[ARRAYSIZE(StatusParts) - 1] -= GetSystemMetrics(SM_CXVSCROLL);
            for (int i = ARRAYSIZE(StatusParts) - 2; i >= 0; --i)
            {
                StatusParts[i] = StatusParts[i + 1] - StatusSize[i];
            }
            GetStatusWnd().SetParts(ARRAYSIZE(StatusParts), StatusParts);
        }

        return 0;
    }

    virtual LRESULT OnDropFiles(HDROP hDrop) override
    {
        UINT Count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        // for (UINT I = 0; I < Count; ++I)
        // {
        // }
        if (Count > 0)
        {
            TCHAR FileName[MAX_PATH];
            DragQueryFile(hDrop, 0, FileName, MAX_PATH);
            Load(FileName);
        }
        DragFinish(hDrop);
        return 0;
    }

    virtual LRESULT OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override
    {
        switch (ID)
        {
        case ID_FILE_OPEN:
            {
                std::tstring Filter; // _T("All Files;*.*;");
                Filter += _T("All Files");
                Filter += SEP;
                Filter += _T("*.*");
                Filter += SEP;

                GetFilter(Filter, SEP, FreeImage_FIFSupportsReading);

                TCHAR FileName[MAX_PATH] = _T("");
                _tcscpy_s(FileName, MAX_PATH, m_Doc.GetFileName());

                OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                ofn.hwndOwner = GetHWND();
                ofn.hInstance = g_hInstance;
                ofn.lpstrFilter = Filter.c_str();
                ofn.nFilterIndex = 1;
                ofn.lpstrFile = FileName;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;

                if (GetOpenFileName(&ofn) != 0)
                {
                    Load(ofn.lpstrFile);
                }
            }
            break;

        case ID_FILE_SAVE:
            m_Doc.Save();
            break;

        case ID_FILE_SAVE_AS:
            {
                TCHAR FileName[MAX_PATH] = _T("");
                _tcscpy_s(FileName, MAX_PATH, m_Doc.GetFileName());

                std::tstring Filter;
                int Index = GetFilter(Filter, SEP, FreeImage_FIFSupportsWriting, FreeImage_GetFIFFromFilename(FileName));

                OPENFILENAME ofn = { sizeof(OPENFILENAME) };
                ofn.hwndOwner = GetHWND();
                ofn.hInstance = g_hInstance;
                // ofn.lpstrFilter = "Windows Bitmap\0*.BMP\0JPEG Standard\0*.JPE;*.JPG;*.JPEG;*.JFIF;*.JIF\0Portable Bitmap\0*.PBM\0Portable Graymap\0*.PGM\0Portable Network Graphics file\0*.PNG\0Portable Pixelmap\0*.PPM\0Tagged Image File Format\0*.TIF;*.TIFF\0";
                ofn.lpstrFilter = Filter.c_str();
                ofn.lpstrFile = FileName;
                ofn.nFilterIndex = Index;
                ofn.nMaxFile = MAX_PATH;
                ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_SHAREAWARE | OFN_HIDEREADONLY;

                if (GetSaveFileName(&ofn) != 0)
                {
                    //FREE_IMAGE_FORMAT format = FreeImage_GetFIFByIndex(ofn.nFilterIndex - 1);
                    //FREE_IMAGE_FORMAT format = static_cast<FREE_IMAGE_FORMAT>(ofn.nFilterIndex - 1);
                    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(ofn.lpstrFile);
                    if (ofn.nFileExtension == 0)
                    {
                        format = GetFilterFromIndex(FreeImage_FIFSupportsWriting, ofn.nFilterIndex);
                        _tcscat_s(ofn.lpstrFile, MAX_PATH, _T("."));
                        _tcscat_s(ofn.lpstrFile, MAX_PATH, FreeImage_GetFormatFromFIF(format));
                    }
                    else if (ofn.lpstrFile[ofn.nFileExtension] == '\0')
                    {
                        format = GetFilterFromIndex(FreeImage_FIFSupportsWriting, ofn.nFilterIndex);
                        _tcscpy_s(ofn.lpstrFile + ofn.nFileExtension, MAX_PATH - ofn.nFileExtension, FreeImage_GetFormatFromFIF(format));
                    }
                    m_Doc.SaveAs(ofn.lpstrFile, format);
                }
            }
            break;

        case ID_FILE_REVERT:
            {
                TCHAR FileName[MAX_PATH];
                _tcscpy_s(FileName, MAX_PATH, m_Doc.GetFileName());
                Load(FileName);
            }
            break;

        case ID_FILE_CLOSE:
            Load(NULL);
            break;

        case ID_FILE_EXIT:
            PostMessage(WM_CLOSE);
            break;

        case ID_EDIT_COPY:
            m_Doc.CopyToClipboard(*this);
            break;

        case ID_EDIT_PASTE:
            m_Doc.PasteFromClipboard(*this);
            break;

        case ID_IMAGE_CONVERT_TO8BITS:
            m_Doc.ConvertTo8Bits();
            break;

        case ID_IMAGE_CONVERT_TO16BITS:
            m_Doc.ConvertTo16Bits();
            break;

        case ID_IMAGE_CONVERT_TO24BITS:
            m_Doc.ConvertTo24Bits();
            break;

        case ID_IMAGE_CONVERT_TO32BITS:
            m_Doc.ConvertTo32Bits();
            break;

        case ID_IMAGE_COLOURQUANTIZE:
            {
                ColourQuantizeDlg dlg;
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.ColorQuantize(dlg.GetMethod());
            }
            break;

        case ID_IMAGE_DITHER:
            {
                DitherDlg dlg;
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.Dither(dlg.GetMethod());
            }
            break;

        case ID_IMAGE_RESCALE:
            {
                RescaleDlg dlg(m_Doc.GetWidth(), m_Doc.GetHeight());
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                     m_Doc.Rescale(dlg.GetWidth(), dlg.GetHeight(), dlg.GetMethod());
            }
            break;

        case ID_IMAGE_ROTATE:
            {
                DoubleDlg dlg(IDD_ROTATE, IDC_ANGLE);
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.Rotate(dlg.GetDouble());
            }
            break;

        case ID_FLIP_HORIZONTAL:
            {
                m_Doc.FlipHorizontal();
            }
            break;

        case ID_FLIP_VERTICAL:
            {
                m_Doc.FlipHorizontal();
            }
            break;

        case ID_IMAGE_INVERT:
            {
                m_Doc.Invert();
            }
            break;

        case ID_IMAGE_BRIGHTNESS:
            {
                DoubleDlg dlg(IDD_BRIGHTNESS, IDC_PERCENTAGE);
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.Brightness(dlg.GetDouble());
            }
            break;

        case ID_IMAGE_CONTRAST:
            {
                DoubleDlg dlg(IDD_CONTRAST, IDC_PERCENTAGE);
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.Contrast(dlg.GetDouble());
            }
            break;

        case ID_IMAGE_GAMMA:
            {
                DoubleDlg dlg(IDD_GAMMA, IDC_GAMMA);
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                    m_Doc.Gamma(dlg.GetDouble());
            }
            break;

        case ID_IMAGE_RECOLOUR:
            {
                RecolourDlg dlg;
                if (dlg.DoModal(GetHWND(), g_hInstance) == IDOK)
                {
                    m_Doc.Recolour(dlg.GetSourceColour(), dlg.GetDestColour(), dlg.GetTolerance());
                }
            }
            break;

        case ID_VIEW_TOOLBAR:
            // TODO Move into FrameWindow or maybe ToolBarWndChain
            GetToolBarWnd().ShowWindow(GetToolBarWnd().IsWindowVisible() ? SW_HIDE : SW_SHOW);
            SizeView();
            break;

        case ID_VIEW_STATUS_BAR:
            // TODO Move into FrameWindow or maybe StatusBarWndChain
            GetStatusWnd().ShowWindow(GetStatusWnd().IsWindowVisible() ? SW_HIDE : SW_SHOW);
            SizeView();
            break;

        case ID_PAGE_NEXT:
            m_Doc.NextPage();
            SendMessage(WM_COMMAND, ID_VIEW_ZOOM_TOFIT);
            break;

        case ID_PAGE_PREV:
            m_Doc.PrevPage();
            SendMessage(WM_COMMAND, ID_VIEW_ZOOM_TOFIT);
            break;

        case ID_HELP_ABOUT:
            {
                ImgViewerAboutDlg ad(g_hInstance);
                ad.DoModal(GetHWND(), g_hInstance);
            }
            break;

        default:
            return Base::OnCommand(NotifyCode, ID, hWnd);
        }
        return 0;
    }

    LRESULT OnMessage(UINT Message, WPARAM wParam, LPARAM lParam) override
    {
        if (Message == WM_CLIPBOARDUPDATE)
        {
            UpdateToolBarStatus();
        }
        return Base::OnMessage(Message, wParam, lParam);
    }

public: // CImgViewerDocListener
    void ImgViewerDocMsg(CImgViewerDoc* pDoc, int Msg) override
    {
        assert(pDoc == &m_Doc);
        if ((Msg & IVDL_NAME_CHANGED) || (Msg & IVDL_MODIFIED_CHANGED))
        {
            if (m_Doc.IsValid())
            {
                LPCTSTR FileName = m_Doc.GetFileName();
                LPCTSTR FileNameStart = FileName + _tcslen(FileName);
                while (FileNameStart > FileName)
                {
                    if (*FileNameStart == '\\')
                    {
                        ++FileNameStart;
                        break;
                    }
                    --FileNameStart;
                }

                TCHAR WndTitle[1024];
                _tcscpy_s(WndTitle, ARRAYSIZE(WndTitle), FileNameStart);
                if (m_Doc.IsModified())
                    _tcscat_s(WndTitle, ARRAYSIZE(WndTitle), _T("*"));
                _tcscat_s(WndTitle, ARRAYSIZE(WndTitle), _T(" - "));
                _tcscat_s(WndTitle, ARRAYSIZE(WndTitle), APP_NAME);
                SetWindowText(WndTitle);
            }
            else
            {
                SetWindowText(APP_NAME);
            }

            UpdateToolBarStatus();
        }

        if (Msg & IVDL_SIZE_CHANGED)
        {
            if (GetStatusWnd().IsAttached())
            {
                if (m_Doc.IsValid())
                {
                    TCHAR Text[1024];

                    _stprintf_s(Text, 1024, _T("%d x %d x %d"), m_Doc.GetWidth(), m_Doc.GetHeight(), m_Doc.GetBPP());
                    GetStatusWnd().SetText(Text, 0, 3);

                    int ColorType = m_Doc.GetColorType();
                    if (ColorType == FIC_RGB)
                        _tcscpy_s(Text, 1024, _T("RGB"));
                    else if (ColorType == FIC_PALETTE)
                        _tcscpy_s(Text, 1024, _T("PALETTE"));
                    else if (ColorType == FIC_RGBALPHA)
                        _tcscpy_s(Text, 1024, _T("RGBA"));
                    else if (ColorType == FIC_CMYK)
                        _tcscpy_s(Text, 1024, _T("CMYK"));
                    else
                        _tcscpy_s(Text, 1024, _T(""));
                    GetStatusWnd().SetText(Text, 0, 2);

                    _stprintf_s(Text, 1024, _T("%d/%d"), m_Doc.GetPage() + 1, m_Doc.GetNumPages());
                    GetStatusWnd().SetText(Text, 0, 1);
                }
                else
                {
                    GetStatusWnd().SetText(_T(""), 0, 1);
                    GetStatusWnd().SetText(_T(""), 0, 2);
                    GetStatusWnd().SetText(_T(""), 0, 3);
                }
            }

            UpdateToolBarStatus(); // When page changes
        }
    }

    void ImgViewerDocError(CImgViewerDoc* /*pDoc*/, const std::string& Msg) override
    {
        MessageBoxA(GetHWND(), Msg.c_str(), APP_NAME_A, MB_OK | MB_ICONERROR);
    }

public: // CCommandStatus
    CommandStatus::State GetCommandStatus(UINT CommandID) override
    {
        CommandStatus::State Status = {};
        Status.Known = true;

        switch (CommandID)
        {
        case ID_FILE_CLOSE:
            if (!m_Doc.IsValid())
                Status.Grayed = true;
            break;

        case ID_FILE_SAVE:
            if (!m_Doc.IsValid() || !m_Doc.IsModified() || !m_Doc.CanSave())
                Status.Grayed = true;
            break;

        case ID_FILE_SAVE_AS:
            if (!m_Doc.IsValid())
                Status.Grayed = true;
            break;

        case ID_FILE_REVERT:
            if (!m_Doc.IsValid() || !m_Doc.IsModified() || (m_Doc.GetFileName()[0] == '\0'))
                Status.Grayed = true;
            break;

        case ID_EDIT_COPY:
            if (!m_Doc.IsValid())
                Status.Grayed = true;
            break;

        case ID_EDIT_PASTE:
            if (!m_Doc.CanPasteFromClipboard(*this))
                Status.Grayed = true;
            break;

        case ID_IMAGE_CONVERT_TO8BITS:
        case ID_IMAGE_CONVERT_TO16BITS:
        case ID_IMAGE_CONVERT_TO24BITS:
        case ID_IMAGE_CONVERT_TO32BITS:
        case ID_IMAGE_COLOURQUANTIZE:
        case ID_IMAGE_DITHER:
        case ID_IMAGE_RESCALE:
        case ID_IMAGE_ROTATE:
        case ID_FLIP_HORIZONTAL:
        case ID_FLIP_VERTICAL:
        case ID_IMAGE_INVERT:
        case ID_IMAGE_BRIGHTNESS:
        case ID_IMAGE_CONTRAST:
        case ID_IMAGE_GAMMA:
        case ID_IMAGE_RECOLOUR:
            if (!m_Doc.IsValid())
                Status.Grayed = true;
            break;

        case ID_VIEW_ZOOM_INCREASE:
        case ID_VIEW_ZOOM_ORIGINAL:
        case ID_VIEW_ZOOM_DECREASE:
        case ID_VIEW_ZOOM_TOFIT:
            if (!m_Doc.IsValid())
                Status.Grayed = true;
            break;

        case ID_PAGE_NEXT:
            Status.Grayed = !m_Doc.IsValid() || (m_Doc.GetPage() + 1) >= m_Doc.GetNumPages();
            break;

        case ID_PAGE_PREV:
            Status.Grayed = !m_Doc.IsValid() || m_Doc.GetPage() <= 0;
            break;

        case ID_VIEW_TOOLBAR: // TODO Move GetCommandStatus into FrameWindow???
            // TODO Move into FrameWindow or maybe ToolBarWndChain
            if (GetToolBarWnd().IsAttached() && GetToolBarWnd().IsWindowVisible())
                Status.Checked = true;
            break;

        case ID_VIEW_STATUS_BAR:
            // TODO Move into FrameWindow or maybe StatusBarWndChain
            if (GetStatusWnd().IsAttached() && GetStatusWnd().IsWindowVisible())
                Status.Checked = true;
            break;

        default:
            Status.Known = false;
            break;
        }

        return Status;
    }

private:
    CImgViewerDoc m_Doc;
};

int CALLBACK wWinMain(
    _In_ HINSTANCE hInstance,
    _In_ HINSTANCE /*hPrevInstance*/,
    _In_ LPTSTR     lpCmdLine,
    _In_ int       nCmdShow
)
{
    InitCommonControls();

    TCHAR filename[MAX_PATH] = _T("");

    LPTSTR     lpCmdLineNext = lpCmdLine;
    while (*lpCmdLineNext != _T('\0'))
    {
        switch (*lpCmdLineNext)
        {
        case _T(' '):
        case _T('\t'):
        case _T('\r'):
        case _T('\b'):
            ++lpCmdLineNext;
            break;

        case _T('\"'):
            {
                // TODO Better parsing if it contains _T('\"') in the string
                LPTSTR     lpBegin = lpCmdLineNext + 1;
                LPTSTR     lpEnd = _tcschr(lpBegin, _T('\"'));
                if (lpEnd != nullptr)
                {
                    _tcsncpy_s(filename, lpBegin, lpEnd - lpBegin);
                    filename[lpEnd - lpBegin] = _T('\0');
                    lpCmdLineNext = lpEnd + 1;
                }
                else
                {
                    _tcscpy_s(filename, lpBegin);
                    lpCmdLineNext += _tcslen(lpCmdLineNext);
                }
            }
            break;

        default:
            {
                LPTSTR     lpBegin = lpCmdLineNext;
                LPTSTR     lpEnd = _tcschr(lpBegin, _T(' '));
                if (lpEnd != nullptr)
                {
                    _tcsncpy_s(filename, lpBegin, lpEnd - lpBegin);
                    filename[lpEnd - lpBegin] = _T('\0');
                    lpCmdLineNext = lpEnd + 1;
                }
                else
                {
                    _tcscpy_s(filename, lpBegin);
                    lpCmdLineNext += _tcslen(lpCmdLineNext);
                }
            }
            break;
        }
    }

    g_hInstance = hInstance;
    g_hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));

    ImgViewerFrame* w = new ImgViewerFrame;
    w->CreateWnd(hInstance, APP_NAME);
    w->ShowWindow(nCmdShow);
    if (filename[0] != _T('\0'))
        w->Load(filename);

    return (int) DoMessageLoop(w->GetHWND(), g_hAccel);
}
