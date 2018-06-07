//#pragma once

#include "FrameWindow.h"

#include <Rad\Rect.h>

#include "WindowView.h"

#include "Chain\CommandStatusChain.h"
#include "Chain\ToolBarWndChain.h"
#include "Chain\StatusWndChain.h"
#include "Chain\MenuToolBarChain.h"

namespace rad
{
    FrameWindow::FrameWindow()
        : m_View(nullptr)
    {
        Add(CommandStatusChain::GetInstance());
        Add(&m_ToolBarWndListener);
        Add(&m_StatusWndChain);
        Add(&m_MenuToolBarListener);
    }

    void FrameWindow::CreateChildren(LPCREATESTRUCT CreateStruct)
    {
        m_View->CreateWnd(CreateStruct->hInstance, _T(""), *this);
        //m_View->SetFocus();
    }

    LRESULT FrameWindow::OnCreate(LPCREATESTRUCT CreateStruct)
    {
        LRESULT res = Window::OnCreate(CreateStruct);

        CreateChildren(CreateStruct);

        GetStatusWnd().SetText(TEXT(""), SBT_NOBORDERS);    // TODO Move into StatusWndChain

        // TODO Move to MenuToolBarChain
        m_MenuToolBarListener.SetMenu(GetMenu());
        if (GetToolBarWnd().IsAttached())
            m_MenuToolBarListener.SetToolBarWnd(GetToolBarWnd());
        //m_MenuToolBarListener.SetAccelerator(hAccel);

        UpdateToolBarStatus();

        return res;
    }

    LRESULT FrameWindow::OnSize(UINT Type, int cx, int cy)
    {
        LRESULT ret = Window::OnSize(Type, cx, cy);

        SizeView();

        return ret;
    }

    LRESULT FrameWindow::OnCommand(WORD NotifyCode, WORD ID, HWND hWnd)
    {
        // TODO Need an easier way to forward the last message to another window
        LRESULT ret = m_View->SendMessage(WM_COMMAND, MAKEWPARAM(ID, NotifyCode), (LPARAM) hWnd);
        if (ret != 0)
            ret = Window::OnCommand(NotifyCode, ID, hWnd);
        return ret;
    }

    RECT FrameWindow::GetViewRect()
    {
        RECT Rect;
        GetClientRect(&Rect);
        if (GetToolBarWnd().IsAttached() && GetToolBarWnd().IsWindowVisible())
        {
            RECT ToolBarRect;
            GetToolBarWnd().GetWindowRect(&ToolBarRect);
            Rect.top += GetHeight(ToolBarRect);
        }
        if (GetStatusWnd().IsAttached() && GetStatusWnd().IsWindowVisible())
        {
            RECT StatusRect;
            GetStatusWnd().GetWindowRect(&StatusRect);
            Rect.bottom -= GetHeight(StatusRect);
        }
        return Rect;
    }

    void FrameWindow::SizeView()
    {
        if (m_View)
        {
            RECT Rect = GetViewRect();

            m_View->MoveWindow(Rect.left, Rect.top, GetWidth(Rect), GetHeight(Rect), true);
            m_View->InvalidateRect();
        }
    }
};
