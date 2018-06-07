#pragma once

#include <Rad\GUI\Window.h>

namespace rad
{
    class WindowView : public Window
    {
    public:
        virtual WindowCreate GetWindowCreate(HINSTANCE hInstance) override;
    };
}
