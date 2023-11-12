#pragma once

#include "ScreenManager.fwd.h"
#include "PlayerElement.fwd.h"

#include "Playerctl.h"
#include "Ueberzug.h"

#include <string>
#include <vector>

using namespace OnePlayer::Playerctl;
using namespace OnePlayer::Ueberzug;

namespace OnePlayer
{

    enum class ContentAlign
    {
        Start,
        Center,
        End
    };

    class PlayerElement
    {
    public:
        PlayerElement(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);

        virtual ~PlayerElement();

        virtual void Update(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        virtual void Update(const MetaData& metaData);
        virtual void Draw(const Window& window, const XYPair& cellsSize,
            const XYPair& cellPos, bool terminalResized = false);
        virtual void HandleClick();

    protected:
        ContentAlign _xAlign;
        ContentAlign _yAlign;
        std::string _content;
    };

    class PlayPauseButton : public PlayerElement
    {
    public:
        ~PlayPauseButton();
        PlayPauseButton(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        void Update(const MetaData& metaData);
        void HandleClick();

    private:
        std::string _status;

        void UpdateContent();
    };

    class NextButton : public PlayerElement
    {
    public:
        using PlayerElement::PlayerElement;
        void Update(const MetaData& metaData);
        void HandleClick();
    };

    class PrevButton : public PlayerElement
    {
    public:
        using PlayerElement::PlayerElement;
        void Update(const MetaData& metaData);
        void HandleClick();
    };

    class TimeLine : public PlayerElement
    {
    public:
        using PlayerElement::PlayerElement;
        void Update(const MetaData& metaData);
        void HandleClick();
        void Draw(const Window& window, const XYPair& cellsSize,
            const XYPair& cellPos, bool terminalResized = false);

    private:
        float _currentPosition;
    };

    class Time : public PlayerElement
    {
    public:
        using PlayerElement::PlayerElement;
        void Update(const MetaData& metaData);
    };

    class Artist : public PlayerElement
    {
    public:
        ~Artist();
        Artist(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        void Update(const MetaData& metaData);
    };

    class Album : public PlayerElement
    {
    public:
        ~Album();
        Album(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        void Update(const MetaData& metaData);
    };

    class Title : public PlayerElement
    {
    public:
        ~Title();
        Title(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        void Update(const MetaData& metaData);
    };

    class Cover : public PlayerElement
    {
    public:
        Cover(
            const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign);
        void Update(const MetaData& metaData);
        void Draw(const Window& window, const XYPair& cellSize,
            const XYPair& cellPos, bool terminalResized = false);

    private:
        class Ueberzug _ueberzug;
        std::string _currentCover;
        std::string _displayedCover;
    };

}
