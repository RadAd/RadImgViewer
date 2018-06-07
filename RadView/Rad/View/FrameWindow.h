#pragma once

#include <Rad\GUI\Window.h>

#include "Chain\CommandStatusChain.h"
#include "Chain\ToolBarWndChain.h"
#include "Chain\StatusWndChain.h"
#include "Chain\MenuToolBarChain.h"

namespace rad
{
    class WindowView;

    class FrameWindow : public Window
    {
    public:
        FrameWindow();

        void SetAccelerator(HACCEL hAccel)
        {
            m_MenuToolBarListener.SetAccelerator(hAccel);
        }

        void SetView(WindowView* v)
        {
            m_View = v;
        }

        ToolBarWnd& GetToolBarWnd() { return m_ToolBarWndListener.m_ToolBar; }
        StatusWnd& GetStatusWnd() { return m_StatusWndChain.m_Status; }

        void UpdateToolBarStatus()
        {
            CommandStatus* cs = dynamic_cast<CommandStatus*>(this);
            if (cs != nullptr)
                m_ToolBarWndListener.UpdateToolBarStatus(cs);
        }

    protected:
        virtual LRESULT OnCreate(LPCREATESTRUCT CreateStruct) override;
        virtual LRESULT OnSize(UINT Type, int cx, int cy) override;
        virtual LRESULT OnCommand(WORD NotifyCode, WORD ID, HWND hWnd) override;

    protected:
        RECT GetViewRect();
        void SizeView();
        virtual void CreateChildren(LPCREATESTRUCT /*CreateStruct*/);

    private:
        WindowView* m_View;

        ToolBarWndChain m_ToolBarWndListener;
        StatusWndChain m_StatusWndChain;
        MenuToolBarChain m_MenuToolBarListener;
    };
}
