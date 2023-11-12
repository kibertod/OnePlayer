#include "ScreenManager.h"
#include "PlayerElement.h"
#include "Playerctl.h"
#include "ScreenManager.fwd.h"

#include <csignal>
#include <curses.h>
#include <iostream>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ncurses.h>
#include <cmath>
#include <csignal>

namespace OnePlayer
{
    XYPair termSize;

    void resizeHandler(int signum)
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        termSize.x = w.ws_col;
        termSize.y = w.ws_row;
        endwin();
        refresh();
        (void)signum;
    }

    XYPair getTerminalSize() { return XYPair(termSize.x, termSize.y); }

    XYPair::XYPair() :
        x(0.0f),
        y(0.0f) {};

    XYPair::XYPair(float _x, float _y) :
        x(_x),
        y(_y) {};

    bool XYPair::operator==(XYPair other)
    {
        return x == other.x && y == other.y;
    };

    Screen::Screen(Layout&& layout) :
        _termSize(),
        _layout(layout)
    {
        std::signal(SIGWINCH, resizeHandler);
        resizeHandler(0);
        initscr();
        curs_set(0);
    }

    void Screen::DrawWindows()
    {
        XYPair size = getTerminalSize();

        clear();

        _termSize.x = size.x;
        _termSize.y = size.y;

        for (auto& window : _layout)
        {
            window.Draw(size, size != _termSize || _termSize == XYPair(0, 0));
        }

        refresh();
    }

    void Screen::Update()
    {
        for (auto& window : _layout)
        {
            window.Update(MetaData(""));
        }
    };

    Window::Window(
        PlayerElement& element, bool hasBorder, XYPair size, XYPair pos) :
        NcursesWin(newwin(0, 0, 0, 0)),
        Element(element),
        Size(size),
        Pos(pos),
        HasBorder(hasBorder)
    {
    }

    void Window::Draw(const XYPair& termSize, bool terminalResized)
    {
        XYPair cellPos;
        XYPair cellsSize;
        cellPos.x = termSize.x * Pos.x;
        cellPos.y = termSize.y * Pos.y;
        cellsSize.x = termSize.x * Size.x;
        cellsSize.y = termSize.y * Size.y;

        wborder(NcursesWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        delwin(NcursesWin);

        NcursesWin = newwin(cellsSize.y, cellsSize.x, cellPos.y, cellPos.x);
        refresh();

        if (HasBorder)
            box(NcursesWin, 0, 0);

        Element.Draw(*this, cellsSize, cellPos, terminalResized);

        wrefresh(NcursesWin);
    }

    void Window::Update(const MetaData& metaData, bool hasBorder)
    {
        HasBorder = hasBorder;
        Element.Update(metaData);
    }

    void Window::Update(const MetaData& metaData) { Element.Update(metaData); }

}
