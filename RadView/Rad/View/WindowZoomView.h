#pragma once

#include "WindowScrollView.h"

namespace rad
{
    class WindowZoomView : public WindowScrollView
    {
    public:
        WindowZoomView();

        SIZE GetViewSize() const override;

        double GetZoom() const
        {
            return m_Zoom;
        }

        void SetZoom(double Zoom);

    private:
        double GetScrollPercent(int nBar) const;
        void SetScrollPercent(int nBar, double Percent);

    private:
        double				m_Zoom;
    };
}
