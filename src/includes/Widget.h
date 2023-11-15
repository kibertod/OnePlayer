#pragma once

#include "Variable.fwd.h"
#include "Widget.fwd.h"
#include "Ueberzug.fwd.h"
#include <curses.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace OnePlayer
{
    enum class Unit
    {
        Pixel,
        Percent
    };

    struct Size
    {
        size_t value;
        Unit unit;

        Size();
        Size(size_t value, Unit unit = Unit::Pixel);

        bool operator==(Size other);
    };

    struct Vec2
    {
        Size x;
        Size y;
        Vec2();
        Vec2(Size x, Size y);

        bool operator==(Vec2 other);
    };

    class Widget
    {
    public:
        bool Visible;
        bool HasBorder;
        Vec2 Size;
        Vec2 StartPadding;
        Vec2 EndPadding;

        Widget();
        Widget(Vec2 size, Vec2 startPadding, Vec2 endPadding, bool hasBorder,
            bool visible, VariableManager& variableManager);
        virtual ~Widget();

        virtual void UpdateVariables() = 0;
        virtual void UpdateSize(
            Vec2 pos, Vec2 space, bool forceRedraw = false) = 0;
        virtual void HandleClick(Vec2 pos) = 0;

    protected:
        virtual void Draw(Vec2 pos, Vec2 space) = 0;
        WINDOW* _ncursesWin;
        Vec2 _lastPos;
        Vec2 _lastSpace;
        VariableManager& _variableManager;
    };

    class Box : public Widget
    {
    public:
        enum class Orientation
        {
            Vertical,
            Horizontal
        };

        Orientation Orientation;

        template<class T, class... Args>
        std::shared_ptr<T> AddChild(Args&&... args)
        {
            _children.push_back(std::make_shared<T>(args...));
            return std::dynamic_pointer_cast<T>(_children.back());
        }

        Box(Vec2 size, Vec2 startPadding, Vec2 endPadding, bool hasBorder,
            bool visible, VariableManager& variableManager) :
            Widget(size, startPadding, endPadding, hasBorder, visible,
                variableManager),
            Orientation(Orientation::Horizontal) {};

        void ReserveChildren(size_t size);
        void UpdateVariables() override;
        void UpdateSize(
            Vec2 pos, Vec2 space, bool forceRedraw = false) override;
        void HandleClick(Vec2 pos) override;

    protected:
        void Draw(Vec2 pos, Vec2 space) override;
        std::vector<std::shared_ptr<Widget>> _children;
    };

    class Text : public Widget
    {
    public:
        enum class ContentAlign
        {
            Start,
            Center,
            End
        };

        ContentAlign XAlign;
        ContentAlign YAlign;

        Text(Vec2 size, Vec2 startPadding, Vec2 endPadding, bool hasBorder,
            bool visible, VariableManager& variableManager,
            const std::string& content) :
            Widget(size, startPadding, endPadding, hasBorder, visible,
                variableManager),
            _clickAction("")
        {
            SetContent(content);
        };

        void SetContent(const std::string& content);
        void SetClickAction(const std::string& action);
        void UpdateVariables() override;
        void UpdateSize(
            Vec2 pos, Vec2 space, bool forceRedraw = false) override;
        void HandleClick(Vec2 pos) override;

    protected:
        void Draw(Vec2 pos, Vec2 space) override;
        void Draw(Vec2 pos, Vec2 space, bool forceClear, Vec2 offset);
        std::string _clickAction;
        std::vector<std::string> _content;
    };

    class Scale : public Text
    {
    public:
        enum class Type
        {
            Horizontal,
            Vertical,
            Circular
        };

        Type Type;
        std::string ValueVar;

        Scale(Vec2 size, Vec2 startPadding, Vec2 endPadding, bool hasBorder,
            bool visible, VariableManager& variableManager,
            const std::string& content, const std::string valueVar) :
            Text(size, startPadding, endPadding, hasBorder, visible,
                variableManager, content),
            Type(Type::Horizontal),
            ValueVar(valueVar) {};

    protected:
        void Draw(Vec2 pos, Vec2 space) override;
    };

    class Image : public Widget

    {
    public:
        std::string ValueVar;
        std::string ClickAction;

        Image(Vec2 size, Vec2 startPadding, Vec2 endPadding, bool hasBorder,
            bool visible, VariableManager& variableManager, Ueberzug& ueberzug,
            const std::string& valueVar) :
            Widget(size, startPadding, endPadding, hasBorder, visible,
                variableManager),
            ValueVar(valueVar),
            _ueberzug(ueberzug),
            _imgSrc("") {};

        void UpdateVariables() override;
        void UpdateSize(
            Vec2 pos, Vec2 space, bool forceRedraw = false) override;
        void HandleClick(Vec2 pos) override;

    protected:
        void Draw(Vec2 pos, Vec2 space) override;
        Ueberzug& _ueberzug;
        std::string _imgSrc;
    };

}
