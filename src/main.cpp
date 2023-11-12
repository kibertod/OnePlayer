#include <chrono>
#include <cstdio>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>
#include <curses.h>
#include <iostream>
#include <ncurses.h>

#include "PlayerElement.h"
#include "ScreenManager.h"
#include "Ueberzug.h"

int main(int argc, char** argv)
{
    using namespace OnePlayer;
    setlocale(LC_ALL, "");
    initscr();

    Layout layout;

    MetaData metaData("");
    PlayPauseButton playPause(
        metaData, ContentAlign::Center, ContentAlign::Center);
    NextButton playNext(metaData, ContentAlign::Start, ContentAlign::Center);
    PrevButton playPrev(metaData, ContentAlign::End, ContentAlign::Center);
    Artist artist(metaData, ContentAlign::Center, ContentAlign::Center);
    Album album(metaData, ContentAlign::Center, ContentAlign::Center);
    Title title(metaData, ContentAlign::Center, ContentAlign::Center);
    TimeLine timeLine(metaData, ContentAlign::Center, ContentAlign::Center);
    Time time(metaData, ContentAlign::Center, ContentAlign::Center);
    Cover cover(metaData, ContentAlign::End, ContentAlign::End);

    layout.emplace_back(title, true, XYPair(1, 0.1f), XYPair(0, 0));
    layout.emplace_back(artist, true, XYPair(0.5f, 0.1f), XYPair(0, 0.1f));
    layout.emplace_back(album, true, XYPair(0.5f, 0.1f), XYPair(0.5f, 0.1f));
    layout.emplace_back(
        playPause, false, XYPair(0.34f, 0.2f), XYPair(0.33f, 0.8f));
    layout.emplace_back(playPrev, false, XYPair(0.33f, 0.2f), XYPair(0, 0.8f));
    layout.emplace_back(
        playNext, false, XYPair(0.33f, 0.2f), XYPair(0.67f, 0.8f));
    layout.emplace_back(timeLine, true, XYPair(0.75f, 0.1f), XYPair(0, 0.7f));
    layout.emplace_back(time, true, XYPair(0.25f, 0.1f), XYPair(0.75f, 0.7f));
    layout.emplace_back(cover, true, XYPair(1, 0.5f), XYPair(0, 0.2f));

    Screen screen(std::move(layout));

    screen.DrawWindows();

    while (true)
    {
        auto point = std::chrono::steady_clock::now();
        point += std::chrono::milliseconds(250);
        screen.Update();
        screen.DrawWindows();
        std::this_thread::sleep_until(point);
    }
    endwin();

    getchar();

    return 0;

    (void)argc;
    (void)argv;
}
