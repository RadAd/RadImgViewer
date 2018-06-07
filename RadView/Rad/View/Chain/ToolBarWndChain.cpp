#include "ToolBarWndChain.h"

#include <Rad\GUI\Window.h>

#include "CommandStatusChain.h"

namespace rad
{
    void ToolBarWndChain::UpdateToolBarStatus(CommandStatus* StatusI)
    {
        assert(StatusI);
        if (m_ToolBar.IsAttached())
        {
            int	NumButton = m_ToolBar.GetButtonCount();
            for (int I = 0; I < NumButton; ++I)
            {
                TBBUTTON	Button = {};
                m_ToolBar.GetButton(I, &Button);

                UpdateToolBarStatus(StatusI, Button.idCommand);
            }
        }
    }

    void ToolBarWndChain::UpdateToolBarStatus(CommandStatus* StatusI, UINT ID)
    {
        CommandStatus::State Status = StatusI->GetCommandStatus(ID);

        int OrigState = m_ToolBar.GetState(ID);
        int	State = OrigState;

        Set(State, !Status.Known, TBSTATE_HIDDEN);
        Set(State, !Status.Grayed, TBSTATE_ENABLED);
        Set(State, Status.Checked, TBSTATE_CHECKED);

        if (State != OrigState)
            m_ToolBar.SetState(ID, State);
    }

    LRESULT ToolBarWndChain::OnNotify(Window* Window, int CtrlID, LPNMHDR Header)
    {
        if (m_ToolBar.IsAttached() && CtrlID == GetDlgCtrlID(m_ToolBar.GetHWND()) && Header->code == TBN_GETINFOTIP)
        {
            HINSTANCE hInstance = (HINSTANCE) Window->GetWindowLongPtr(GWLP_HINSTANCE);
            LPNMTBGETINFOTIP	TBGetInfoTip = (LPNMTBGETINFOTIP) Header;
            LoadString(hInstance, TBGetInfoTip->iItem, TBGetInfoTip->pszText, TBGetInfoTip->cchTextMax);
        }
        return DoDefault(Window);
    }

    LRESULT ToolBarWndChain::OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        if (m_ToolBar.IsAttached() && Message == WM_SIZE)
            m_ToolBar.SendMessage(Message, wParam, lParam);

        switch (Message)
        {
        case WM_NOTIFY: return OnNotify(Window, (int) wParam, (LPNMHDR) lParam);
        default:        return WindowChain::OnMessage(Window, Message, wParam, lParam);
        }
    }
}
