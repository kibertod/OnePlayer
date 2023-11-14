#include "Widget.h"
#include "Variable.h"
#include <algorithm>
#include <codecvt>
#include <cstddef>
#include <curses.h>
#include <iostream>
#include <locale>
#include <string>
#include <sstream>
#include <unistd.h>

namespace OnePlayer
{
    Size::Size() :
        value(0),
        unit(Unit::Pixel) {};

    Size::Size(size_t value, Unit unit) :
        value(value),
        unit(unit)
    {
    }

    bool Size::operator==(Size other)
    {
        return value == other.value && unit == other.unit;
    }

    Vec2::Vec2() {};

    Vec2::Vec2(Size x, Size y) :
        x(x),
        y(y) {};

    bool Vec2::operator==(Vec2 other) { return x == other.x && y == other.y; }

    Widget::Widget(Vec2 size, Vec2 startPadding, Vec2 endPadding,
        bool hasBorder, bool visible, VariableManager& variableManager) :
        Visible(visible),
        HasBorder(hasBorder),
        Size(size),
        StartPadding(startPadding),
        EndPadding(endPadding),
        _ncursesWin(newwin(0, 0, 0, 0)),
        _variableManager(variableManager)
    {
    }

    Widget::~Widget() { }

    void Box::ReserveChildren(size_t size) { _children.reserve(size); }

    void Box::UpdateVariables()
    {
        for (auto& child : _children)
            child->UpdateVariables();
    }

    void Box::UpdateSize(Vec2 pos, Vec2 space, bool forceRedraw)
    {
        if (_lastPos == pos && _lastSpace == space && !forceRedraw)
            return;

        _lastSpace = space;
        _lastPos = pos;

        wborder(_ncursesWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');

        wclear(_ncursesWin);
        delwin(_ncursesWin);

        _ncursesWin =
            newwin(space.y.value, space.x.value, pos.y.value, pos.x.value);
        wresize(_ncursesWin, space.y.value, space.x.value);
        if (HasBorder)
            box(_ncursesWin, 0, 0);

        refresh();
        wrefresh(_ncursesWin);
        Draw(pos, space);
    }

    void Box::HandleClick(Vec2 pos)
    {
        if (pos.x.value > _lastSpace.x.value + _lastPos.x.value ||
            pos.y.value > _lastSpace.y.value + _lastPos.y.value ||
            pos.x.value < _lastPos.x.value || pos.y.value < _lastPos.y.value)
            return;

        for (auto& child : _children)
            child->HandleClick(pos);
    }

    void Box::Draw(Vec2 pos, Vec2 space)
    {
        if (HasBorder)
        {
            pos.x.value += 1;
            pos.y.value += 1;
            space.x.value -= 2;
            space.y.value -= 2;
        }

        size_t offset = 0;

        for (auto& child : _children)
        {
            Vec2 childPos = pos;
            Vec2 childSpace = space;

            switch (Orientation)
            {
            case (Box::Orientation::Vertical):
                switch (child->Size.y.unit)
                {
                case (Unit::Pixel):
                    childSpace.y.value = child->Size.y.value;
                    childSpace.x.value =
                        std::min(child->Size.x.value, space.x.value);
                    break;

                case (Unit::Percent):
                    childSpace.y.value =
                        space.y.value * child->Size.y.value / 100;
                    childSpace.x.value =
                        std::min(space.x.value * child->Size.x.value / 100,
                            space.x.value);
                    break;
                }

                childSpace.y.value =
                    std::min(childSpace.y.value, space.y.value - offset);
                childPos.y.value += offset;
                offset += childSpace.y.value;
                break;

            case (Box::Orientation::Horizontal):
                switch (child->Size.x.unit)
                {
                case (Unit::Pixel):
                    childSpace.x.value = child->Size.x.value;
                    childSpace.y.value =
                        std::min(child->Size.y.value, space.y.value);
                    break;

                case (Unit::Percent):
                    childSpace.x.value =
                        space.x.value * child->Size.x.value / 100;
                    childSpace.y.value =
                        std::min(space.y.value * child->Size.y.value / 100,
                            space.y.value);
                    break;
                }

                childSpace.x.value =
                    std::min(childSpace.x.value, space.x.value - offset);
                childPos.x.value += offset;
                offset += childSpace.x.value;
                break;
            }

            child->UpdateSize(childPos, childSpace, true);
        }
    }

    void Text::SetContent(const std::string& contentRef)
    {
        std::string content(contentRef);
        size_t linesCount = std::count(content.begin(), content.end(), '\n');
        for (size_t i = 0; i < linesCount + 1; i++)
        {
            size_t endline = content.find("\n");
            if (endline == content.npos)
            {
                content += "\n";
                endline = content.size() - 1;
            }

            std::string line = content.substr(0, endline);
            _content.push_back(line);
            content = content.substr(endline + 1);
        }

        _content.shrink_to_fit();
    }

    void Text::SetClickAction(const std::string& action)
    {
        _clickAction = action;
    }

    void Text::UpdateVariables() { Draw(_lastPos, _lastSpace); }

    void Text::UpdateSize(Vec2 pos, Vec2 space, bool forceRedraw)
    {
        if (pos == _lastPos && space == _lastSpace && !forceRedraw)
            return;

        _lastSpace = space;
        _lastPos = pos;

        wborder(_ncursesWin, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
        wclear(_ncursesWin);
        delwin(_ncursesWin);

        _ncursesWin =
            newwin(space.y.value, space.x.value, pos.y.value, pos.x.value);
        wresize(_ncursesWin, space.y.value, space.x.value);

        if (HasBorder)
            box(_ncursesWin, 0, 0);

        Draw(pos, space);
    }

    void Text::HandleClick(Vec2 pos)
    {
        if (pos.x.value > _lastSpace.x.value + _lastPos.x.value ||
            pos.y.value > _lastSpace.y.value + _lastPos.y.value ||
            pos.x.value < _lastPos.x.value || pos.y.value < _lastPos.y.value)
            return;

        if (_clickAction == "")
            return;

        pid_t pid = fork();
        if (pid == 0)
        {
            execlp(_clickAction.substr(0, _clickAction.find(" ")).data(),
                _clickAction.data());
            exit(0);
        }
    }

    void Text::Draw(Vec2 pos, Vec2 space)
    {
        if (HasBorder)
        {
            pos.y.value += 1;
            pos.x.value += 1;
            space.x.value -= 2;
            space.y.value -= 2;
        }

        for (int i = 0; i < static_cast<int>(space.y.value); i++)
        {
            for (int j = 0; j < static_cast<int>(space.x.value); j++)
                mvwprintw(_ncursesWin, i + static_cast<int>(HasBorder),
                    j + static_cast<int>(HasBorder), " ");
        }

        size_t yOffset = HasBorder;

        switch (YAlign)
        {
        case (Text::ContentAlign::Start):
            yOffset += StartPadding.y.value;
            break;
        case (Text::ContentAlign::Center):
            yOffset +=
                (space.y.value - StartPadding.y.value - EndPadding.y.value) /
                    2 -
                _content.size() / 2;
            break;
        case (Text::ContentAlign::End):
            yOffset += space.y.value - EndPadding.y.value - _content.size();
        }

        for (const auto& lineRef : _content)
        {
            std::string line(lineRef);

            while (line.find("${") != line.npos)
            {
                size_t startLiteral = line.find("${");
                size_t endLiteral = line.find("}");
                std::string variableName = line.substr(
                    startLiteral + 2, endLiteral - startLiteral - 2);
                line = line.substr(0, startLiteral) +
                       _variableManager.GetValue(variableName) +
                       line.substr(endLiteral + 1, line.size() - 1);
            }

            size_t xOffset = HasBorder;

            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> m_cvt;
            switch (XAlign)
            {
            case (Text::ContentAlign::Start):
                xOffset += StartPadding.x.value;
                break;
            case (Text::ContentAlign::Center):
                xOffset += (space.x.value - StartPadding.x.value -
                               EndPadding.x.value) /
                               2 -
                           m_cvt.from_bytes(line).length() / 2;
                break;
            case (Text::ContentAlign::End):
                xOffset += space.x.value - EndPadding.x.value -
                           m_cvt.from_bytes(line).length();
                break;
            }

            mvwprintw(_ncursesWin, yOffset, xOffset, "%s", line.data());
            yOffset++;
        }
        refresh();
        wrefresh(_ncursesWin);
    }

}
