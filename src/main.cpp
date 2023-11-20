#include <cstddef>
#include <sys/ioctl.h>
#include <chrono>
#include <cstdio>
#include <curses.h>
#include <thread>
#include <iostream>
#include <unistd.h>

#include "Ueberzug.h"
#include "Variable.h"
#include "Widget.h"

int main(int argc, char** argv)
{
    using namespace OnePlayer;
    setlocale(LC_ALL, "");

    VariableManager variableManager;
    Ueberzug ueberzug;

    WINDOW* screen = initscr();
    keypad(screen, true);
    mousemask(ALL_MOUSE_EVENTS, nullptr);
    curs_set(0);

    Box box(Vec2(20, 20), Vec2(0, 0), Vec2(0, 0), false, true, variableManager);
    box.Orientation = Box::Orientation::Vertical;

    std::shared_ptr<Box> hBox =
        box.AddChild<Box>(Vec2(Size(100, Unit::Percent), 5), Vec2(0, 0),
            Vec2(0, 0), false, true, variableManager);

    std::shared_ptr<Text> title = hBox->AddChild<Text>(
        Vec2(Size(50, Unit::Percent), Size(100, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager,
        "Title:   ${title}\n"
        "Artist:  ${artist}\n"
        "Album:   ${album}\n"
        "Котёнок: ${polycat} =D");
    title->XAlign = Text::ContentAlign::Start;
    title->YAlign = Text::ContentAlign::Start;

    std::shared_ptr<Image> cover =
        hBox->AddChild<Image>(Vec2(10'000, 5), Vec2(0, 0), Vec2(0, 0), false,
            true, variableManager, ueberzug, "cover");
    cover->XAlign = Image::ContentAlign::End;
    cover->YAlign = Image::ContentAlign::Start;
    cover->ClickAction = "playerctl play-pause";

    std::shared_ptr<Box> vBox = box.AddChild<Box>(
        Vec2(Size(100, Unit::Percent), Size(25, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager);
    vBox->Orientation = Box::Orientation::Vertical;

    std::shared_ptr<Box> controls = vBox->AddChild<Box>(
        Vec2(Size(100, Unit::Percent), Size(2, Unit::Pixel)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager);
    std::shared_ptr<Text> prev = controls->AddChild<Text>(
        Vec2(2, 2), Vec2(0, 0), Vec2(0, 0), false, true, variableManager, "<");
    prev->ClickAction = "playerctl previous";
    std::shared_ptr<Text> playPause = controls->AddChild<Text>(Vec2(2, 2),
        Vec2(0, 0), Vec2(0, 0), false, true, variableManager, "${play-pause}");
    playPause->ClickAction = "playerctl play-pause";
    std::shared_ptr<Text> next = controls->AddChild<Text>(
        Vec2(2, 2), Vec2(0, 0), Vec2(0, 0), false, true, variableManager, ">");
    next->ClickAction = "playerctl next";
    std::shared_ptr<Scale> album = controls->AddChild<Scale>(
        Vec2(Size(80, Unit::Percent), Size(2, Unit::Pixel)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager, "", "position");
    album->Type = Scale::Type::Horizontal;

    variableManager.AddVariable("title", "playerctl -p ncspot metadata title",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("position",
        "python ~/.config/eww/scripts/mediaplayer.py get_time",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("artist", "playerctl -p ncspot metadata artist",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("album", "playerctl -p ncspot metadata album",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("play-pause",
        "python ~/.config/eww/scripts/mediaplayer.py playing",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("cover",
        "playerctl -p ncspot metadata mpris:artUrl", Variable::Type::Poll, 0.5);
    variableManager.AddVariable("polycat", "polycat", Variable::Type::Stdin);

    struct winsize termSize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    box.UpdateSize(Vec2(0, 0), Vec2(termSize.ws_col, termSize.ws_row), true);

    timeout(1);
    while (true)
    {
        auto point = std::chrono::steady_clock::now();
        point += std::chrono::milliseconds(1000 / 60);

        ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);

        if (box.Size.x.value != termSize.ws_col ||
            box.Size.y.value != termSize.ws_row)
        {
            box.UpdateSize(
                Vec2(0, 0), Vec2(termSize.ws_col, termSize.ws_row), true);
            box.Size = Vec2(termSize.ws_col, termSize.ws_row);
        }

        if (variableManager.HasUpdate())
            box.UpdateVariables();

        int key = getch();
        if (key == KEY_MOUSE)
        {
            MEVENT event;
            if (getmouse(&event) == OK)
            {
                box.HandleClick(Vec2(event.x, event.y));
            }
        }

        std::this_thread::sleep_until(point);
    }

    return 0;

    (void)argc;
    (void)argv;
}
