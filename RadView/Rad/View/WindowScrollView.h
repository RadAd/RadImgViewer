#pragma once

#include "WindowView.h"

namespace rad
{
    class WindowScrollView : public WindowView
    {
    public:
        WindowScrollView(bool TrackUpdate = true);

        void UpdateScrollBars();

    protected:
        virtual SIZE GetDocSize() const = 0;
        virtual SIZE GetViewSize() const { return GetDocSize(); }

    protected:
        LRESULT OnCreate(LPCREATESTRUCT CreateStruct) override;
        LRESULT OnEraseBackground(DevContextRef DC) override;
        LRESULT OnHScroll(HWND ScrollBar, int Code, int Pos) override;
        LRESULT OnVScroll(HWND ScrollBar, int Code, int Pos) override;
        LRESULT OnSize(UINT Type, int cx, int cy) override;

    private:
        void OnScroll(int Bar, int Code, int DefaultPage);

    protected:
        /*virtual*/ void OnScrollBarUpdate(int Bar);

    private:
        bool			m_TrackUpdate;
        bool			m_UpdatingScrollBars;
    };
}
