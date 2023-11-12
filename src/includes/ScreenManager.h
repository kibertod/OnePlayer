#pragma once

#include <string>
#include <vector>

#include "PlayerElement.fwd.h"
#include "Playerctl.h"
#include "ScreenManager.fwd.h"

typedef struct _win_st WINDOW;

namespace OnePlayer
{
    struct XYPair
    {
        float x;
        float y;

        XYPair();
        XYPair(float _x, float _y);

        bool operator==(XYPair other);
    };

    class Window
    {
    public:
        WINDOW* NcursesWin;
        PlayerElement& Element;
        const XYPair Size, Pos;
        bool HasBorder;

        Window(PlayerElement& element, bool hasBorder, XYPair size, XYPair pos);
        void Draw(const XYPair& termSize, bool terminalResized = false);
        void Update(const Playerctl::MetaData& metaData);
        void Update(const Playerctl::MetaData& metaData, bool hasBorder);
    };

    using Layout = std::vector<Window>;

    class Screen
    {
    public:
        Screen(Layout&& layout);
        void DrawWindows();
        void Update();

    private:
        XYPair _termSize;
        Layout _layout;
    };

}
