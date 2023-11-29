#include "Widget.h"
#include "Variable.h"
#include "Ueberzug.h"
#include <algorithm>
#include <cmath>
#include <codecvt>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <curses.h>
#include <functional>
#include <iostream>
#include <locale>
#include <stdexcept>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>

std::vector<std::string> split(std::string target)
{
    size_t pieces = std::count(target.begin(), target.end(), ' ');

    std::vector<std::string> res;
    res.reserve(pieces + 1);

    for (size_t i = 0; i < pieces + 1; i++)
    {
        res.emplace_back(target.substr(0, target.find(" ")));
        target = target.substr(target.find(" ") + 1);
    }

    return res;
}

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

    void Widget::HandleClick(Vec2 pos)
    {
        if (pos.x.value > _lastSpace.x.value + _lastPos.x.value ||
            pos.y.value > _lastSpace.y.value + _lastPos.y.value ||
            pos.x.value < _lastPos.x.value || pos.y.value < _lastPos.y.value)
            return;

        if (ClickAction == "")
            return;

        std::vector<std::string> command = split(ClickAction);
        char** commandArr = new char*[command.size() + 1];
        for (size_t i = 0; i < command.size(); i++)
        {
            commandArr[i] = command[i].data();
        }
        commandArr[command.size()] = nullptr;

        pid_t pid = fork();
        if (pid == 0)
        {
            setlocale(LC_ALL, "");
            execvp(command[0].c_str(), const_cast<char* const*>(commandArr));
        }
        delete[] commandArr;
    }

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

        size_t expandedChildrenCount = 0;
        size_t totalExplicitSize = 0;

        for (auto& child : _children)
        {
            switch (Orientation)
            {
            case Box::Orientation::Vertical:
                if (child->Size.y.value == 0)
                    expandedChildrenCount++;
                else
                {
                    if (child->Size.y.unit == Unit::Pixel)
                        totalExplicitSize += child->Size.y.value;
                    else
                        totalExplicitSize +=
                            space.y.value * child->Size.y.value / 100;
                }
                break;
            case Box::Orientation::Horizontal:
                if (child->Size.x.value == 0)
                    expandedChildrenCount++;
                else
                {
                    if (child->Size.x.unit == Unit::Pixel)
                        totalExplicitSize += child->Size.x.value;
                    else
                        totalExplicitSize +=
                            space.x.value * child->Size.x.value / 100;
                }
                break;
            }
        }

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
                    break;

                case (Unit::Percent):
                    childSpace.y.value =
                        space.y.value * child->Size.y.value / 100;
                    break;
                }

                switch (child->Size.x.unit)
                {
                case (Unit::Pixel):
                    childSpace.x.value =
                        std::min(child->Size.x.value, space.x.value);
                    break;
                case (Unit::Percent):
                    childSpace.x.value =
                        std::min(space.x.value * child->Size.x.value / 100,
                            space.x.value);
                    break;
                }

                switch (AlterAlignment)
                {
                case Widget::ContentAlign::Start:
                    break;
                case Widget::ContentAlign::Center:
                    childPos.x.value +=
                        (space.x.value - childSpace.x.value) / 2;
                    break;
                case Widget::ContentAlign::End:
                    childPos.x.value += space.x.value - childSpace.x.value;
                }

                if (space.y.value > totalExplicitSize &&
                    child->Size.y.value == 0)
                {
                    childSpace.y.value = (space.y.value - totalExplicitSize) /
                                         expandedChildrenCount;
                }
                else
                {
                    childSpace.y.value =
                        std::min(childSpace.y.value, space.y.value - offset);
                }

                if (child->Size.x.value == 0)
                    childSpace.x.value = space.x.value;

                childPos.y.value += offset;
                offset += childSpace.y.value;
                break;

            case (Box::Orientation::Horizontal):
                switch (child->Size.x.unit)
                {
                case (Unit::Pixel):
                    childSpace.x.value = child->Size.x.value;
                    break;

                case (Unit::Percent):
                    childSpace.x.value =
                        space.x.value * child->Size.x.value / 100;
                    break;
                }
                switch (child->Size.y.unit)
                {
                case (Unit::Pixel):
                    childSpace.y.value =
                        std::min(child->Size.y.value, space.y.value);
                    break;

                case (Unit::Percent):
                    childSpace.y.value =
                        std::min(space.y.value * child->Size.y.value / 100,
                            space.y.value);
                    break;
                }

                switch (AlterAlignment)
                {
                case Widget::ContentAlign::Start:
                    break;
                case Widget::ContentAlign::Center:
                    childPos.y.value +=
                        (space.y.value - childSpace.y.value) / 2;
                    break;
                case Widget::ContentAlign::End:
                    childPos.y.value += space.y.value - childSpace.y.value;
                }

                if (space.x.value > totalExplicitSize &&
                    child->Size.x.value == 0)
                {
                    childSpace.x.value = (space.x.value - totalExplicitSize) /
                                         expandedChildrenCount;
                }
                else
                {
                    childSpace.x.value =
                        std::min(childSpace.x.value, space.x.value - offset);
                }

                if (child->Size.y.value == 0)
                    childSpace.y.value = space.y.value;

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

    void Text::Draw(Vec2 pos, Vec2 space, bool forceClear, Vec2 offset)
    {
        if (HasBorder)
        {
            pos.y.value += 1;
            pos.x.value += 1;
            space.x.value -= 2;
            space.y.value -= 2;
        }

        if (forceClear)
            for (int i = 0; i < static_cast<int>(space.y.value); i++)
            {
                for (int j = 0; j < static_cast<int>(space.x.value); j++)
                    mvwprintw(_ncursesWin, i + static_cast<int>(HasBorder),
                        j + static_cast<int>(HasBorder), " ");
            }

        size_t yOffset = HasBorder + offset.y.value;

        switch (YAlign)
        {
        case (Widget::ContentAlign::Start):
            yOffset += StartPadding.y.value;
            break;
        case (Widget::ContentAlign::Center):
            yOffset +=
                (space.y.value - StartPadding.y.value - EndPadding.y.value) /
                    2 -
                _content.size() / 2;
            break;
        case (Widget::ContentAlign::End):
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

            size_t xOffset = HasBorder + offset.x.value;

            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> m_cvt;

            if (m_cvt.from_bytes(line).length() > space.x.value)
                line = m_cvt.to_bytes(m_cvt.from_bytes(line).substr(
                           0, space.x.value - 3)) +
                       "...";

            switch (XAlign)
            {
            case (Widget::ContentAlign::Start):
                xOffset += StartPadding.x.value;
                break;
            case (Widget::ContentAlign::Center):
                xOffset += (space.x.value - StartPadding.x.value -
                               EndPadding.x.value) /
                               2 -
                           m_cvt.from_bytes(line).length() / 2;
                break;
            case (Widget::ContentAlign::End):
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

    void Text::Draw(Vec2 pos, Vec2 space)
    {
        Text::Draw(pos, space, true, Vec2());
    }

    void Scale::Draw(Vec2 pos, Vec2 space)
    {
        size_t value;

        std::vector<std::string> initialContent = _content;

        try
        {
            value = std::stoul(_variableManager.GetValue(ValueVar));
        }
        catch (std::exception&)
        {
            value = 0;
        }

        switch (Type)
        {
        case (Scale::Type::Horizontal):
        {
            _content = std::vector<std::string>();
            _content.reserve(1);
            std::stringstream stream;
            for (size_t i = 0; i < space.x.value - HasBorder * 2; i++)
            {
                if (static_cast<float>(i) / space.x.value * 100 <= value)
                    stream << "█";
                else
                    stream << "░";
            }
            _content.emplace_back(stream.str());
            Text::Draw(pos, space);
            _content = initialContent;
            break;
        }
        case (Scale::Type::Vertical):
        {
            size_t maxValue = space.y.value - HasBorder * 2;
            _content = std::vector<std::string>();
            _content.reserve(maxValue);
            for (int i = maxValue - 1; i >= 0; i--)
            {
                if (static_cast<float>(i) / maxValue * 100 <= value)
                    _content.emplace_back("█");
                else
                    _content.emplace_back("░");
            }
            Text::Draw(pos, space);
            _content = initialContent;
            break;
        }
        case (Scale::Type::Circular):
        {
            if (HasBorder)
            {
                space.y.value -= 2;
                space.x.value -= 2;
                pos.y.value += 1;
                pos.x.value += 1;
            }

            size_t maxValue = space.y.value * 2 + space.x.value * 2;
            size_t firstSection = space.x.value / 2 + space.x.value % 2;
            size_t secondSection = firstSection + space.y.value;
            size_t thirdSection = secondSection + space.x.value;
            size_t fourthSection = thirdSection + space.y.value;
            size_t fifthSection = fourthSection + (space.x.value) / 2;

            std::vector<std::u32string> u32Content;
            u32Content.reserve(space.y.value);
            for (size_t i = 0; i < space.y.value; i++)
                u32Content.emplace_back(space.x.value, ' ');

            for (size_t i = 0; i < maxValue; i++)
            {
                char32_t block;
                if (static_cast<float>(i) / maxValue * 100 <= value)
                    block = 9608;
                else
                    block = 9617;
                (void)fifthSection;

                if (i < firstSection)
                    u32Content[0][(space.x.value) / 2 + i] = block;
                else if (i < secondSection)
                {
                    u32Content[i - firstSection][space.x.value - 1] = block;
                    u32Content[i - firstSection][space.x.value - 2] = block;
                }
                else if (i < thirdSection)
                    u32Content[space.y.value - 1]
                              [space.x.value - (i - secondSection)] = block;
                else if (i < fourthSection)
                {
                    u32Content[space.y.value - (i - thirdSection) - 1][0] =
                        block;
                    u32Content[space.y.value - (i - thirdSection) - 1][1] =
                        block;
                }
                else if (i < fifthSection)
                    u32Content[0][i - fourthSection] = block;
            }

            _content = std::vector<std::string>();
            _content.reserve(space.y.value - HasBorder * 2);
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> m_cvt;
            for (const auto& u32line : u32Content)
                _content.emplace_back(m_cvt.to_bytes(u32line));

            if (HasBorder)
            {
                space.y.value += 2;
                space.x.value += 2;
                pos.y.value -= 1;
                pos.x.value -= 1;
            }

            Text::Draw(pos, space, true, Vec2());
            _content = initialContent;

            space.y.value -= 2;
            space.x.value -= 4;
            Text::Draw(pos, space, false, Vec2(2, 1));
            break;
        }
        }
    }

    void Scale::HandleClick(Vec2 pos)
    {
        if (pos.x.value > _lastSpace.x.value + _lastPos.x.value ||
            pos.y.value > _lastSpace.y.value + _lastPos.y.value ||
            pos.x.value < _lastPos.x.value || pos.y.value < _lastPos.y.value)
            return;

        if (ClickAction == "")
            return;

        float posOnScale = static_cast<float>(pos.x.value - _lastPos.x.value) /
                           _lastSpace.x.value;

        std::string clickActionBuffer = ClickAction;

        if (ClickAction.find("{.}") != ClickAction.npos)
            ClickAction.replace(
                ClickAction.find("{.}"), 3, std::to_string(posOnScale * 100));
        if (ClickAction.find("{%}") != ClickAction.npos)
            ClickAction.replace(
                ClickAction.find("{%}"), 3, std::to_string(posOnScale * 100));

        Widget::HandleClick(pos);

        ClickAction = clickActionBuffer;
    }

    void Image::UpdateVariables()
    {
        std::string updatedSrc = _variableManager.GetValue(ValueVar);

        if (updatedSrc == "-" || updatedSrc == _imgSrc)
            return;

        _ueberzug.RemoveImage(updatedSrc);
        _imgSrc = updatedSrc;
        Draw(_lastPos, _lastSpace);
    }

    void Image::UpdateSize(Vec2 pos, Vec2 space, bool forceRedraw)
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

        refresh();
        wrefresh(_ncursesWin);

        Draw(pos, space);
    }

    void Image::Draw(Vec2 pos, Vec2 space)
    {
        if (HasBorder)
        {
            pos.x.value += 1;
            pos.y.value += 1;
            space.x.value -= 2;
            space.y.value -= 2;
        }

        std::string imagePath;
        if (_imgSrc.starts_with("https://") || _imgSrc.starts_with("http://"))
        {
            imagePath = _ueberzug.CacheImage(_imgSrc);
        }
        else
            imagePath = _imgSrc;

        if (space.x.value > space.y.value * 2)
            switch (XAlign)
            {
            case Image::ContentAlign::Start:
                break;
            case Image::ContentAlign::Center:
                pos.x.value += (space.x.value - space.y.value * 2) / 2;
                break;
            case Image::ContentAlign::End:
                pos.x.value += space.x.value - space.y.value * 2 - 1;
                break;
            }
        else
            switch (YAlign)
            {
            case Image::ContentAlign::Start:
                break;
            case Image::ContentAlign::Center:
                pos.y.value +=
                    (space.y.value - static_cast<float>(space.x.value) / 2) / 2;
                break;
            case Image::ContentAlign::End:
                pos.y.value +=
                    space.y.value - static_cast<float>(space.x.value) / 2;
                break;
            }

        _ueberzug.AddImage(imagePath, pos, space);
    }

}
