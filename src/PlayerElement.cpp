#include "PlayerElement.h"

#include "ScreenManager.fwd.h"
#include "Playerctl.h"
#include "Ueberzug.h"

#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <curses.h>
#include <algorithm>
#include <unistd.h>
#include <vector>

namespace OnePlayer
{

    using namespace OnePlayer::Playerctl;
    using namespace OnePlayer::Ueberzug;

    PlayerElement::PlayerElement(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        _xAlign(xAlign),
        _yAlign(yAlign),
        _content("--\n")
    {
        Update(metaData);
    }

    PlayerElement::~PlayerElement() {};

    void PlayerElement::Update(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign)
    {
        Update(metaData);
        _xAlign = xAlign;
        _yAlign = yAlign;
    }

    void PlayerElement::Update(const MetaData& metaData) { (void)metaData; }

    void PlayerElement::Draw(const Window& window, const XYPair& cellsSize,
        const XYPair& cellPos, bool terminalResized)
    {
        (void)terminalResized;
        int contentXPos = 0;
        int contentYPos = 0;

        std::vector<std::string> lines;

        int lineStartIndex = 0;
        int longestLineLength = 0;
        for (int i = 0; i < std::count(_content.begin(), _content.end(), '\n');
             i++)
        {
            int lineEndIndex = _content.find("\n", lineStartIndex);

            std::string line =
                _content.substr(lineStartIndex, lineEndIndex - lineStartIndex);
            lines.push_back(line);

            int length = 0;
            char* lineStartPtr = line.data();
            while (*lineStartPtr)
                length += (*lineStartPtr++ & 0xc0) != 0x80;
            if (length > longestLineLength)
                longestLineLength = length;

            lineStartIndex = lineEndIndex + 1;
        }

        XYPair availableSpace = cellsSize;

        if (window.HasBorder)
        {
            contentXPos = 1;
            contentYPos = 1;
            availableSpace.x -= 2;
            availableSpace.y -= 2;
        }

        int linesCount = std::count(_content.begin(), _content.end(), '\n');

        switch (_xAlign)
        {
        case ContentAlign::Start:
            break;
        case ContentAlign::Center:
            contentXPos += static_cast<int>(availableSpace.x) / 2 +
                           static_cast<int>(availableSpace.x) % 2 -
                           longestLineLength / 2 - longestLineLength % 2;
            break;
        case ContentAlign::End:
            contentXPos += availableSpace.x - longestLineLength;
            break;
        }

        switch (_yAlign)
        {
        case ContentAlign::Start:
            break;
        case ContentAlign::Center:
            contentYPos += static_cast<int>(availableSpace.y) / 2 +
                           static_cast<int>(availableSpace.y) % 2 -
                           linesCount / 2 - linesCount % 2;
            break;
        case ContentAlign::End:
            contentYPos += cellsSize.y - linesCount -
                           static_cast<int>(window.HasBorder) * 2;
            break;
        }

        for (int i = 0; i < std::count(_content.begin(), _content.end(), '\n');
             i++)
        {
            mvwprintw(window.NcursesWin, contentYPos + i, contentXPos, "%s",
                lines[i].data());
        }

        (void)cellPos;
    }

    void PlayerElement::HandleClick() {};

    PlayPauseButton::PlayPauseButton(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        PlayerElement(metaData, xAlign, yAlign) {};

    void PlayPauseButton::Update(const MetaData& metaData)
    {
        _status = metaData.Status;
        UpdateContent();
    }

    PlayPauseButton::~PlayPauseButton() { }

    void PlayPauseButton::HandleClick()
    {
        togglePlayPause("");

        if (_status == "Playing")
            _status = "Paused";
        else
            _status = "Playing";

        UpdateContent();
    }

    void PlayPauseButton::UpdateContent()
    {
        if (_status != "Playing")
            _content = " 🬵█████🬱 \n🬻██▔🮃▀🮅█🬺\n🬬██▁▃▄▆█🬝\n 🬊█████🬆 \n";
        else
            _content = " 🬵█████🬱 \n🬻█  █  █🬺\n🬬█  █  █🬝\n 🬊█████🬆 \n";
    }

    void NextButton::Update(const MetaData& metaData)
    {
        (void)metaData;
        _content = "▆▄▂\n🮅▀🮂\n";
    }

    void NextButton::HandleClick() { playNext(); }

    void PrevButton::Update(const MetaData& metaData)
    {
        (void)metaData;
        _content = "▂▄▆\n🮂▀🮅\n";
    }

    void PrevButton::HandleClick() { playPrevious(); }

    void TimeLine::Update(const MetaData& metaData)
    {
        float time, length;

        try
        {
            length = std::stof(metaData.Length) / 1'000'000;
            time = std::stof(metaData.Position);
        }
        catch (std::invalid_argument _)
        {
            _currentPosition = -1;
            return;
        }

        _currentPosition = time / length;
    }

    void TimeLine::HandleClick() { }

    void TimeLine::Draw(const Window& window, const XYPair& cellsSize,
        const XYPair& cellPos, bool terminalResized)
    {
        std::stringstream contentString;

        for (int i = 0; i < cellsSize.x - 3; i++)
        {
            if (i / (cellsSize.x - 3) <= _currentPosition)
                contentString << "█";
            else if (_currentPosition == -1)
                contentString << "-";
            else
                contentString << "▒";
        }
        contentString << "\n";

        _content = contentString.str();

        PlayerElement::Draw(window, cellsSize, cellPos, terminalResized);
    }

    void pasteTimeUnit(std::stringstream& stream, int value, int maxValue,
        bool seconds = false)
    {
        if (maxValue > 0 || seconds)
        {
            if (value < 10)
                stream << "0";

            stream << value;

            if (!seconds)
                stream << ":";
        }
    }

    void Time::Update(const MetaData& metaData)
    {
        float length, position;

        try
        {
            length = std::stof(metaData.Length) / 1'000'000;
            position = std::stof(metaData.Position);
        }
        catch (std::invalid_argument _)
        {
            _content = "--:--/--:--\n";
            return;
        }

        int lengthH = length / 3600;
        int lengthM = length / 60;
        int lengthS = static_cast<int>(length) % 60;

        int positionH = position / 3600;
        int positionM = position / 60;
        int positionS = static_cast<int>(position) % 60;

        std::stringstream contentString;

        pasteTimeUnit(contentString, positionH, lengthH);
        pasteTimeUnit(contentString, positionM, lengthM);
        pasteTimeUnit(contentString, positionS, lengthS, true);
        contentString << "\\";
        pasteTimeUnit(contentString, lengthH, lengthH);
        pasteTimeUnit(contentString, lengthM, lengthM);
        pasteTimeUnit(contentString, lengthS, lengthS, true);
        contentString << "\n";

        _content = contentString.str();
    }

    Artist::Artist(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        PlayerElement(metaData, xAlign, yAlign) {};

    Artist::~Artist() { }

    void Artist::Update(const MetaData& metaData)
    {
        _content = metaData.Artist + "\n";
    }

    Album::Album(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        PlayerElement(metaData, xAlign, yAlign) {};

    Album::~Album() { }

    void Album::Update(const MetaData& metaData)
    {
        _content = metaData.Album + "\n";
    }

    Title::Title(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        PlayerElement(metaData, xAlign, yAlign) {};

    Title::~Title() { }

    void Title::Update(const MetaData& metaData)
    {
        _content = metaData.Title + "\n";
    }

    Cover::Cover(
        const MetaData& metaData, ContentAlign xAlign, ContentAlign yAlign) :
        PlayerElement(metaData, xAlign, yAlign),
        _ueberzug(),
        _currentCover(""),
        _displayedCover("")
    {
    }

    void Cover::Update(const MetaData& metaData)
    {
        _currentCover = metaData.ImageUrl;
    }

    void Cover::Draw(const Window& window, const XYPair& cellSize,
        const XYPair& cellPos, bool terminalResized)
    {
        (void)terminalResized;
        XYPair imagePos = cellPos;
        XYPair imageSize = cellSize;

        if (window.HasBorder)
        {
            imagePos.x++;
            imagePos.y++;
            imageSize.x -= 2;
            imageSize.y -= 2;
        }

        float imageSide = std::min(imageSize.x, imageSize.y * 2);

        imagePos.x += imageSize.x / 2 - imageSide / 2;
        imagePos.y += imageSize.y / 2 - imageSide / 4;

        _ueberzug.ChangeImage(_currentCover, imagePos, imageSize);

        _displayedCover = _currentCover;
    }

}
