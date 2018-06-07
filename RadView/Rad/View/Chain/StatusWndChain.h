#pragma once

#include <Rad\GUI\WindowChain.h>

#include <Rad\GUI\CommCtrl.h>

namespace rad
{
    class StatusWndChain : public WindowChain
    {
    public:
        StatusWnd	m_Status;

    protected:
        virtual LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override;
    };
}
