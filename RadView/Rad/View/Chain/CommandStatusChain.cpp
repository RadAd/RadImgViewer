#include "CommandStatusChain.h"

#include <Rad\GUI\Window.h>
#include <Rad\GUI\Menu.h>

namespace rad
{
    CommandStatusChain* CommandStatusChain::GetInstance()
    {
        static CommandStatusChain csl;
        return &csl;
    }

    LRESULT CommandStatusChain::OnInitMenuPopup(Window* Window, HMENU hMenu, UINT /*Pos*/, BOOL /*SystemMenu*/)
    {
        CommandStatus* StatusI = dynamic_cast<CommandStatus*>(Window);

        Menu	Menu(hMenu);

        int Count = Menu.GetItemCount();
        for (int I = 0; I < Count; ++I)
        {
            UINT	CommandID = Menu.GetItemID(I);
            switch (CommandID)
            {
            case -1: // a popup menu
                break;

            case 0: // a seperator
                break;

            default:
                {
                    assert(StatusI);
                    CommandStatus::State Status = StatusI->GetCommandStatus(CommandID);

                    if (Status.Known)
                    {
                        MENUITEMINFO	mii;
                        mii.cbSize = sizeof(MENUITEMINFO);
                        mii.fMask = MIIM_STATE;
                        Menu.GetItemInfo(CommandID, &mii, false);

                        UINT	State = mii.fState;

                        Set(State, Status.Grayed, MFS_GRAYED);
                        Set(State, Status.Checked, MFS_CHECKED);

                        if (State != mii.fState)
                        {
                            mii.fState = State;
                            Menu.SetItemInfo(CommandID, &mii, false);
                        }
                    }
                }
                break;
            }
        }

        return DoDefault(Window);
    }

    LRESULT CommandStatusChain::OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam)
    {
        switch (Message)
        {
        case WM_INITMENUPOPUP:  return OnInitMenuPopup(Window, (HMENU) wParam, (UINT) LOWORD(lParam), (BOOL) HIWORD(lParam));
        default:                return WindowChain::OnMessage(Window, Message, wParam, lParam);
        }
    }

}
