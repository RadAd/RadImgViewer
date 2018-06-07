#pragma once

#include <Rad\GUI\WindowChain.h>

#include <Rad\GUI\CommCtrl.h>

namespace rad
{
    class CommandStatus;

    class ToolBarWndChain : public WindowChain
    {
    public:
        ToolBarWnd m_ToolBar;

        void UpdateToolBarStatus(CommandStatus* StatusI);

    protected:
        void UpdateToolBarStatus(CommandStatus* StatusI, UINT ID);

    private:
        static void Set(int& State, bool b, int v)
        {
            if (b)
                State |= v;
            else
                State &= ~v;
        }

    protected:
        LRESULT OnNotify(Window* Window, int CtrlID, LPNMHDR Header);

    protected:
        virtual LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override;
    };
}
