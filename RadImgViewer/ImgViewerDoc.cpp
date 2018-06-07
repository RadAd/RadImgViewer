#include "ImgViewerDoc.H"
#include <FreeImage.H>
// AG #include "Rad/Win/GlobalMemory.H"
// AG #include "Rad/Win/Clipboard.H"
#include "Rad/Win/WinHandle.H"
#include "Color.H"
#include <Math.h>

#ifdef UNICODE
#define FreeImage_GetFIFFromFilename FreeImage_GetFIFFromFilenameU
#define FreeImage_Load FreeImage_LoadU
#define FreeImage_Save FreeImage_SaveU
#endif

namespace
{
    BOOL __stdcall MyGlobalFree(HGLOBAL Handle)
    {
        return GlobalFree(Handle) == NULL;
    }

    template<class T>
    class CGlobalLock
    {
    public:
        CGlobalLock(HGLOBAL hMem)
            : hMem(hMem)
        {
            ptr = static_cast<T*>(GlobalLock(hMem));
        }

        ~CGlobalLock()
        {
            GlobalUnlock(hMem);
        }

        HGLOBAL GetHandle() const
        {
            return hMem;
        }

        T* GetPointer()
        {
            return ptr;
        }

    private:
        HGLOBAL hMem;
        T* ptr;
    };

    class CClipboard
    {
    public:
        CClipboard(const rad::WindowProxy& Wnd)
            //: Wnd(Wnd)
        {
            if (!OpenClipboard(Wnd.GetHWND()))
                rad::ThrowWinError(_T(__FUNCTION__));
        }

        ~CClipboard()
        {
            CloseClipboard();
            //if (!CloseClipboard())
                //rad::ThrowWinError(_T(__FUNCTION__));
        }

        void SetData(_In_ UINT uFormat, _In_opt_ HANDLE hMem)
        {
            if (SetClipboardData(uFormat, hMem) == NULL)
                rad::ThrowWinError(_T(__FUNCTION__));
        }

        HGLOBAL GetData(_In_ UINT uFormat)
        {
            HGLOBAL ret = NULL;
            if ((ret = GetClipboardData(uFormat)) == NULL)
                rad::ThrowWinError(_T(__FUNCTION__));
            return ret;
        }

        bool IsFormatAvailable(_In_ UINT uFormat) const
        {
            return IsClipboardFormatAvailable(uFormat) != FALSE;
        }

    private:
        //rad::WindowProxy Wnd;
    };
}

const option Quantize[] =
{
    { FIQ_WUQUANT,  TEXT("Xiaolin Wu") },
    { FIQ_NNQUANT,  TEXT("NeuQuant neural-net") },
    { FIQ_LFPQUANT, TEXT("Lossless Fast Pseudo") },
    { -1, 0 },
};

const option Dither[] =
{
    { FID_FS,			TEXT("Floyd & Steinberg") },
    { FID_BAYER4x4,		TEXT("Bayer(order 2)") },
    { FID_BAYER8x8,		TEXT("Bayer(order 3)") },
    { FID_CLUSTER6x6,	TEXT("Clustered(order 3)") },
    { FID_CLUSTER8x8,	TEXT("Clustered(order 4)") },
    { FID_CLUSTER16x16,	TEXT("Clustered(order 8)") },
    //{ FID_BAYER16x16,	TEXT("Bayer(order 4)") },
    { -1, 0 },
};

const option Rescale[] =
{
    { FILTER_BOX,			TEXT("Fourier b-spline") },
    { FILTER_BICUBIC,		TEXT("Mitchell & Netravali") },
    { FILTER_BILINEAR,		TEXT("Bilinear filter") },
    { FILTER_BSPLINE,		TEXT("Cubic b-spline") },
    { FILTER_CATMULLROM,	TEXT("Catmull-Rom spline") },
    { FILTER_LANCZOS3,		TEXT("Lanczos3 filter") },
    { -1, 0 },
};

static void MyFIOM(FREE_IMAGE_FORMAT /*fif*/, const char *msg)
{
    MessageBoxA(NULL, msg, "ImgViewer", MB_OK | MB_ICONINFORMATION);
}

CImgViewerDoc::CImgViewerDoc()
    : m_Image(NULL), m_Modified(false)
{
    m_FileName[0] = '\0';

    FreeImage_SetOutputMessage(MyFIOM);
}

CImgViewerDoc::~CImgViewerDoc()
{
    Detach();
}

void CImgViewerDoc::AddListener(CImgViewerDocListener *Listener)
{
    m_Listeners.push_back(Listener);
}

void CImgViewerDoc::RemoveListener(CImgViewerDocListener *Listener)
{
    vector <CImgViewerDocListener *>::iterator it;
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
    {
        if ((*it) == Listener)
        {
            m_Listeners.erase(it);
            break;
        }
    }
}

bool CImgViewerDoc::Load(LPCTSTR FileName)
{
    Detach();
    m_Modified = false;

    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(FileName);
    Attach(FreeImage_Load(format, FileName));

    if (IsValid())
        _tcscpy_s(m_FileName, MAX_PATH, FileName);

    Broadcast(CImgViewerDocListener::IVDL_NAME_CHANGED | CImgViewerDocListener::IVDL_MODIFIED_CHANGED
        | CImgViewerDocListener::IVDL_SIZE_CHANGED | CImgViewerDocListener::IVDL_PALETTE_CHANGED
        | CImgViewerDocListener::IVDL_BITS_CHANGED);

    return IsValid();
}

bool CImgViewerDoc::CanSave()
{
    FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilename(m_FileName);
    return FreeImage_FIFSupportsWriting(format) == TRUE;
}

bool CImgViewerDoc::Save()
{
    return SaveAs(m_FileName, FreeImage_GetFIFFromFilename(m_FileName));
}

bool CImgViewerDoc::SaveAs(LPCTSTR FileName, FREE_IMAGE_FORMAT Format)
{
    int flags = 0;
    if (Format == FIF_ICO && GetBPP() == 32)
        flags |= ICO_MAKEALPHA;
    BOOL	Saved = FreeImage_Save(Format, m_Image, FileName, flags);

    if (Saved)
    {
        m_Modified = false;
        int	Msg = CImgViewerDocListener::IVDL_MODIFIED_CHANGED;
        if (FileName != m_FileName)
        {
            _tcscpy_s(m_FileName, MAX_PATH, FileName);
            Msg |= CImgViewerDocListener::IVDL_NAME_CHANGED;
        }
        Broadcast(Msg);
    }

    return Saved == TRUE;
}

void CImgViewerDoc::Close()
{
    Detach();
    Broadcast(CImgViewerDocListener::IVDL_NAME_CHANGED | CImgViewerDocListener::IVDL_MODIFIED_CHANGED
        | CImgViewerDocListener::IVDL_SIZE_CHANGED | CImgViewerDocListener::IVDL_PALETTE_CHANGED
        | CImgViewerDocListener::IVDL_BITS_CHANGED);
}

int CImgViewerDoc::GetWidth() const
{
    return FreeImage_GetWidth(m_Image);
}

int CImgViewerDoc::GetHeight() const
{
    return FreeImage_GetHeight(m_Image);
}

int CImgViewerDoc::GetBPP() const
{
    return FreeImage_GetBPP(m_Image);
}

int CImgViewerDoc::GetColorType() const
{
    return FreeImage_GetColorType(m_Image);
}

void CImgViewerDoc::CopyToClipboard(rad::WindowProxy &Wnd) const
{
    if (IsValid())
    {
        int	Size = FreeImage_GetDIBSize(m_Image);
        rad::WinHandle<HGLOBAL> Mem(GlobalAlloc(GHND | GMEM_MOVEABLE, Size), MyGlobalFree);
        {
            CGlobalLock<char>	Lock(Mem.Get());
            char	*Ptr = Lock.GetPointer();
            BITMAPINFOHEADER *bi = FreeImage_GetInfoHeader(m_Image);
            memcpy(Ptr, bi, sizeof(BITMAPINFOHEADER));
            Ptr += sizeof(BITMAPINFOHEADER);
            int PaletteSize = FreeImage_GetColorsUsed(m_Image) * sizeof(RGBQUAD);
            if (PaletteSize == 0)
                PaletteSize = 3 * sizeof(DWORD);
            else
                memcpy(Ptr, FreeImage_GetPalette(m_Image), PaletteSize);
            Ptr += PaletteSize;
            memcpy(Ptr, FreeImage_GetBits(m_Image), Size - PaletteSize - sizeof(BITMAPINFOHEADER));
        }

        CClipboard	Clip(Wnd);
        Clip.SetData(CF_DIB, Mem.Release());
    }
}

bool CImgViewerDoc::CanPasteFromClipboard(rad::WindowProxy &Wnd) const
{
    CClipboard	Clip(Wnd);
    return Clip.IsFormatAvailable(CF_DIB);
}

void CImgViewerDoc::PasteFromClipboard(rad::WindowProxy &Wnd)
{
    CClipboard	Clip(Wnd);
    CGlobalLock<char>	Lock(Clip.GetData(CF_DIB));
    if (Lock.GetHandle() != NULL)
    {
        char	*Ptr = Lock.GetPointer();
        BITMAPINFOHEADER *bi = (BITMAPINFOHEADER *) Ptr;
        Attach(FreeImage_Allocate(bi->biWidth, bi->biHeight, bi->biBitCount));
        Ptr += sizeof(BITMAPINFOHEADER);
        int PaletteSize = FreeImage_GetColorsUsed(m_Image) * sizeof(RGBQUAD);
        if (PaletteSize == 0)
            PaletteSize = 3 * sizeof(DWORD);
        else
            memcpy(FreeImage_GetPalette(m_Image), Ptr, PaletteSize);
        Ptr += PaletteSize;
        memcpy(FreeImage_GetBits(m_Image), Ptr, FreeImage_GetDIBSize(m_Image) - PaletteSize - sizeof(BITMAPINFOHEADER));

        _tcscpy_s(m_FileName, MAX_PATH, _T("Clipboard"));
        m_Modified = true;

        Broadcast(CImgViewerDocListener::IVDL_NAME_CHANGED | CImgViewerDocListener::IVDL_MODIFIED_CHANGED
            | CImgViewerDocListener::IVDL_SIZE_CHANGED | CImgViewerDocListener::IVDL_PALETTE_CHANGED
            | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
}

void CImgViewerDoc::ConvertTo8Bits()
{
    FIBITMAP *NewImage = FreeImage_ConvertTo8Bits(m_Image);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error converting to 8 bits");
}

void CImgViewerDoc::ConvertTo16Bits()
{
    FIBITMAP *NewImage = FreeImage_ConvertTo16Bits565(m_Image);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error converting to 16 bits");
}

void CImgViewerDoc::ConvertTo24Bits()
{
    FIBITMAP *NewImage = FreeImage_ConvertTo24Bits(m_Image);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error converting to 24 bits");
}

void CImgViewerDoc::ConvertTo32Bits()
{
    FIBITMAP *NewImage = FreeImage_ConvertTo32Bits(m_Image);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error converting to 32 bits");
}

void CImgViewerDoc::ColorQuantize(int option)
{
    FIBITMAP *NewImage = FreeImage_ColorQuantize(m_Image, (FREE_IMAGE_QUANTIZE) option);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error colour quantizing");
}

void CImgViewerDoc::Dither(int option)
{
    FIBITMAP *NewImage = FreeImage_Dither(m_Image, (FREE_IMAGE_DITHER) option);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error dithering");
}

void CImgViewerDoc::Rescale(int width, int height, int option)
{
    FIBITMAP *NewImage = FreeImage_Rescale(m_Image, width, height, (FREE_IMAGE_FILTER) option);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error rescaling");
}

void CImgViewerDoc::Rotate(double angle)
{
    FIBITMAP *NewImage = FreeImage_RotateClassic(m_Image, angle);
    if (NewImage)
    {
        FreeImage_Unload(m_Image);
        m_Image = NewImage;
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED | CImgViewerDocListener::IVDL_SIZE_CHANGED |
            CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error rotating");
}

void CImgViewerDoc::FlipHorizontal()
{
    if (FreeImage_FlipHorizontal(m_Image))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error flipping horizontal");
}

void CImgViewerDoc::FlipVertical()
{
    if (FreeImage_FlipVertical(m_Image))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error flipping vertical");
}

void CImgViewerDoc::Invert()
{
    if (FreeImage_Invert(m_Image))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error inverting");
}

void CImgViewerDoc::Brightness(double percentage)
{
    if (FreeImage_AdjustBrightness(m_Image, percentage))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error adjusting brightness");
}

void CImgViewerDoc::Contrast(double percentage)
{
    if (FreeImage_AdjustContrast(m_Image, percentage))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error adjusting contrast");
}

void CImgViewerDoc::Gamma(double gamma)
{
    if (FreeImage_AdjustGamma(m_Image, gamma))
    {
        m_Modified = true;
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
    else
        BroadcastError("Error adjusting gamma");
}

void CImgViewerDoc::Recolour(const COLORREF& source, const COLORREF& dest, double Tolerance)
{
    ColorRGB	sourcergb(GetRValue(source) / 255.0, GetGValue(source) / 255.0, GetBValue(source) / 255.0);
    ColorHSV	sourcehsv(ToHSV(sourcergb));

    ColorRGB	destrgb(GetRValue(dest) / 255.0, GetGValue(dest) / 255.0, GetBValue(dest) / 255.0);
    ColorHSV	desthsv(ToHSV(destrgb));

    int width = GetWidth();
    int height = GetHeight();

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            RGBQUAD value;
            FreeImage_GetPixelColor(m_Image, x, y, &value);

            ColorRGB	rgb(value.rgbRed / 255.0, value.rgbGreen / 255.0, value.rgbBlue / 255.0);
            ColorHSV	hsv(ToHSV(rgb));
            ColorHSV diff(0, 0, 0);
            diff.h = hsv.h - sourcehsv.h;
            if (fabs(diff.h) < Tolerance)
            {
                // hue
                hsv.h = desthsv.h + diff.h;
                if (hsv.h > 360.0)
                    hsv.h -= 360.0;
                if (hsv.h < 0.0)
                    hsv.h += 360.0;
                // sat
                diff.s = hsv.s - sourcehsv.s;
                hsv.s = desthsv.s + diff.s;
                if (hsv.s > 1.0)
                    hsv.s = 1.0;
                if (hsv.s < 0.0)
                    hsv.s = 0.0;
                // val
                diff.v = hsv.v - sourcehsv.v;
                hsv.v = desthsv.v + diff.v;
                if (hsv.v > 1.0)
                    hsv.v = 1.0;
                if (hsv.v < 0.0)
                    hsv.v = 0.0;

                rgb = ToRGB(hsv);
                value.rgbRed = (BYTE) (rgb.r * 255);
                value.rgbGreen = (BYTE) (rgb.g * 255);
                value.rgbBlue = (BYTE) (rgb.b * 255);

                FreeImage_SetPixelColor(m_Image, x, y, &value);
                m_Modified = true;
            }
        }
    }

    if (m_Modified)
    {
        Broadcast(CImgViewerDocListener::IVDL_MODIFIED_CHANGED |
            CImgViewerDocListener::IVDL_PALETTE_CHANGED | CImgViewerDocListener::IVDL_BITS_CHANGED);
    }
}

HPALETTE CImgViewerDoc::CreatePalette() const
{
    if (IsValid())
    {
        RGBQUAD *pRgb = FreeImage_GetPalette(m_Image);
        unsigned int count = FreeImage_GetColorsUsed(m_Image);

        LOGPALETTE *PaletteInfo = (LOGPALETTE *) new char[sizeof(LOGPALETTE) + count * sizeof(PALETTEENTRY)];
        PaletteInfo->palNumEntries = (WORD) count;

        for (unsigned int i = 0; i < count; i++)
        {
            PaletteInfo->palPalEntry[i].peRed = pRgb[i].rgbRed;
            PaletteInfo->palPalEntry[i].peGreen = pRgb[i].rgbGreen;
            PaletteInfo->palPalEntry[i].peBlue = pRgb[i].rgbBlue;
            PaletteInfo->palPalEntry[i].peFlags = 0;
        }

        HPALETTE	hPalette = ::CreatePalette(PaletteInfo);
        delete[] PaletteInfo;
        return hPalette;
    }
    else
        return NULL;
}

HBITMAP CImgViewerDoc::CreateBitmap(rad::DevContextRef DC) const
{
    if (IsValid())
    {
        HBITMAP Bitmap = CreateDIBitmap(DC.GetHandle(),
            FreeImage_GetInfoHeader(m_Image),
            CBM_INIT,
            FreeImage_GetBits(m_Image),
            FreeImage_GetInfo(m_Image),
            DIB_RGB_COLORS);
        return Bitmap;
    }
    else
        return NULL;
}

void CImgViewerDoc::Attach(FIBITMAP *Image)
{
    Detach();
    m_Image = Image;
}

void CImgViewerDoc::Detach()
{
    if (m_Image)
    {
        FreeImage_Unload(m_Image);
        m_Image = NULL;
    }
    m_Modified = false;
    m_FileName[0] = '\0';
}

void CImgViewerDoc::Broadcast(int Msg)
{
    vector <CImgViewerDocListener *>::iterator it;
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
    {
        (*it)->ImgViewerDocMsg(this, Msg);
    }
}

void CImgViewerDoc::BroadcastError(const std::string& Msg)
{
    vector <CImgViewerDocListener *>::iterator it;
    for (it = m_Listeners.begin(); it != m_Listeners.end(); ++it)
    {
        (*it)->ImgViewerDocError(this, Msg);
    }
}
