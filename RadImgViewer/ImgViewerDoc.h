#ifndef IMGVIEWERDOC_H
#define IMGVIEWERDOC_H

#include <windows.h>

#include <Rad\GUI\WindowProxy.h>
#include <Rad\GUI\GdiObject.h>

#include <vector>

using namespace std;
//using namespace stl;
//using namespace stlport;

struct FIMULTIBITMAP;
struct FIBITMAP;
enum FREE_IMAGE_FORMAT;

struct option
{
    int value;
    LPCTSTR name;
};

extern const option Quantize[];
extern const option Dither[];
extern const option Rescale[];

class CImgViewerDoc;

class CImgViewerDocListener
{
public:
    enum MsgE { IVDL_NONE = 0, IVDL_NAME_CHANGED = 1, IVDL_MODIFIED_CHANGED = 2, IVDL_SIZE_CHANGED = 4, IVDL_PALETTE_CHANGED = 8, IVDL_BITS_CHANGED = 16 };
    virtual void ImgViewerDocMsg(CImgViewerDoc* pDoc, int Msg) = 0;

    // TODO Seperate out into a different interface???
    virtual void ImgViewerDocError(CImgViewerDoc* pDoc, const std::string& Msg) = 0;
};

class CImgViewerDoc
{
public:
    CImgViewerDoc();
    ~CImgViewerDoc();

    void AddListener(CImgViewerDocListener *Listener);
    void RemoveListener(CImgViewerDocListener *Listener);

    bool Load(LPCTSTR FileName);
    bool CanSave();
    bool Save();
    bool SaveAs(LPCTSTR FileName, FREE_IMAGE_FORMAT Format);

    void Close();

    int GetPage();
    int GetNumPages();
    void NextPage();
    void PrevPage();

    int GetWidth() const;
    int GetHeight() const;
    int GetBPP() const;
    int GetColorType() const;

    bool IsValid() const
    {
        return m_Image != NULL;
    }

    bool IsModified() const
    {
        return m_Modified;
    }

    LPCTSTR GetFileName() const
    {
        return m_FileName;
    }

    void CheckFileTime();

    void CopyToClipboard(rad::WindowProxy &Wnd) const;
    bool CanPasteFromClipboard(rad::WindowProxy &Wnd) const;
    void PasteFromClipboard(rad::WindowProxy &Wnd);

    void ConvertTo8Bits();
    void ConvertTo16Bits();
    void ConvertTo24Bits();
    void ConvertTo32Bits();

    void ColorQuantize(int option);
    void Dither(int option);

    void Rescale(int width, int height, int option);
    void Rotate(double angle);
    void FlipHorizontal();
    void FlipVertical();

    void Invert();
    void Brightness(double percentage);
    void Contrast(double percentage);
    void Gamma(double gamma);
    void Recolour(const COLORREF& source, const COLORREF& dest, double Tolerance);

    HPALETTE CreatePalette() const;
    HBITMAP CreateBitmap(rad::DevContextRef DC) const;

protected:
    void Attach(FIMULTIBITMAP* Multi, int Page, FIBITMAP *Image, bool fClearFileName = true);
    void Detach(bool fClearFileName = true);
    void Modified(FIBITMAP *Image);

    void Broadcast(int Msg);
    void BroadcastError(const std::string& Msg);

private:
    FIMULTIBITMAP*  m_Multi;
    int             m_Page;
    FIBITMAP*       m_Image;
    bool		m_Modified;
    TCHAR		m_FileName[MAX_PATH];
    FILETIME    m_ModifiedTime;

    vector <CImgViewerDocListener *> m_Listeners;
};

#endif
