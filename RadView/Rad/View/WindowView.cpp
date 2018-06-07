#include "WindowView.h"

#include <Rad\GUI\WindowCreate.h>

namespace rad
{
    WindowCreate WindowView::GetWindowCreate(HINSTANCE hInstance)
    {
        WindowCreate wc = Window::GetWindowCreate(hInstance);
        wc.Style = WS_CHILD | WS_VISIBLE;
        return wc;
    }
}
