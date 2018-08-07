#pragma once

#include <Rad\GUI\WindowChain.h>

namespace rad
{
    class CommandStatus
    {
    public:
        struct State
        {
            bool Known : 1,
                Grayed : 1,
                Checked : 1;
        };

        virtual State GetCommandStatus(UINT CommandID) = 0;
    };

    class CommandStatusChain : public WindowChain
    {
    private:
        CommandStatusChain()
        {
        }

    public:
        static CommandStatusChain* GetInstance();

    private:
        static void Set(UINT& State, bool b, int v)
        {
            if (b)
                State |= v;
            else
                State &= ~v;
        }

    protected:
        LRESULT OnInitMenuPopup(Window* Window, HMENU hMenu, UINT Pos, BOOL SystemMenu);

    protected:
        virtual LRESULT OnMessage(Window* Window, UINT Message, WPARAM wParam, LPARAM lParam) override;
    };
}
