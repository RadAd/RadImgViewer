#pragma once

#include <Rad\GUI\Dialog.h>

#include <Rad\GUI\GdiObject.h>

// TODO Move into RadView ???
class AboutDlg : public rad::Dialog
{
public:
    AboutDlg(rad::Icon Icon)
        : m_Icon(std::move(Icon))
    {
        //m_Icon = Icon;
    }

    INT_PTR DoModal(HWND hParent, HINSTANCE hInstance);

protected:
    BOOL OnInitDialog(HWND FocusControl) override;
    BOOL OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override;

private:
    rad::Icon		m_Icon;
    TCHAR		m_Version[1024];
    TCHAR		m_Product[1024];
};

class ImgViewerAboutDlg : public AboutDlg
{
public:
    ImgViewerAboutDlg(HINSTANCE hInstance);

    INT_PTR DoModal(HWND hParent, HINSTANCE hInst);

protected:
    virtual BOOL OnInitDialog(HWND FocusControl) override;
};

class ColourQuantizeDlg : public rad::Dialog
{
public:
    INT_PTR DoModal(HWND hParent, HINSTANCE hInst);

    int GetMethod() const
    {
        return m_Method;
    }

protected:
    virtual BOOL OnInitDialog(HWND FocusControl) override;
    virtual BOOL OnOk() override;

private:
    int m_Method;
};

class DitherDlg : public rad::Dialog
{
public:
    INT_PTR DoModal(HWND hParent, HINSTANCE hInst);

    int GetMethod() const
    {
        return m_Method;
    }

protected:
    virtual BOOL OnInitDialog(HWND FocusControl) override;
    virtual BOOL OnOk() override;

private:
    int m_Method;
};

class RescaleDlg : public rad::Dialog
{
public:
    RescaleDlg(int Width, int Height)
        : m_Width(Width), m_Height(Height)
    {
    }

    INT_PTR DoModal(HWND hParent, HINSTANCE hInst);

    int GetWidth() const
    {
        return m_Width;
    }

    int GetHeight() const
    {
        return m_Height;
    }

    int GetMethod() const
    {
        return m_Method;
    }

protected:
    virtual BOOL OnInitDialog(HWND FocusControl) override;
    virtual BOOL OnOk() override;

private:
    int m_Width;
    int m_Height;
    int m_Method;
};

class DoubleDlg : public rad::Dialog
{
public:
    DoubleDlg(int Dialog, int Item)
        : m_Dialog(Dialog), m_Item(Item), m_Double(0.0)
    {
    }

    INT_PTR DoModal(HWND hParent, HINSTANCE hInst)
    {
        return Dialog::DoModal(hInst, m_Dialog, hParent);
    }

    double GetDouble() const
    {
        return m_Double;
    }

protected:
    virtual BOOL OnOk() override;

private:
    int m_Dialog;
    int m_Item;
    double m_Double;
};

class RecolourDlg : public rad::Dialog
{
public:
    RecolourDlg()
    {
        m_SourceColor = RGB(255, 0, 0);
        m_DestColor = RGB(0, 255, 0);
        m_Tolerance = 5.0;
        m_SourceBrush.Create(m_SourceColor);
        m_DestBrush.Create(m_DestColor);
    }

    INT_PTR DoModal(HWND hParent, HINSTANCE hInst);

    const COLORREF& GetSourceColour() const
    {
        return m_SourceColor;
    }

    const COLORREF& GetDestColour() const
    {
        return m_DestColor;
    }

    double GetTolerance() const
    {
        return m_Tolerance;
    }

protected:
    virtual BOOL OnInitDialog(HWND FocusControl) override;
    virtual BOOL OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override;
    virtual BOOL OnMessage(UINT Message, WPARAM wParam, LPARAM lParam) override;
    virtual BOOL OnOk() override;

private:
    COLORREF    m_SourceColor;
    COLORREF    m_DestColor;
    double      m_Tolerance;
    rad::Brush  m_SourceBrush;
    rad::Brush  m_DestBrush;
};
