#include <sys/ioctl.h>
#include <chrono>
#include <cstdio>
#include <curses.h>
#include <thread>
#include <iostream>

#include "Ueberzug.h"
#include "Variable.h"
#include "Widget.h"

int main(int argc, char** argv)
{
    using namespace OnePlayer;
    setlocale(LC_ALL, "");

    VariableManager variableManager;
    Ueberzug ueberzug;

    initscr();
    curs_set(0);

    Box box(Vec2(20, 20), Vec2(0, 0), Vec2(0, 0), false, true, variableManager);
    box.Orientation = Box::Orientation::Vertical;

    std::shared_ptr<Box> hBox =
        box.AddChild<Box>(Vec2(Size(100, Unit::Percent), 6), Vec2(0, 0),
            Vec2(0, 0), false, true, variableManager);

    std::shared_ptr<Text> title = hBox->AddChild<Text>(
        Vec2(Size(75, Unit::Percent), Size(100, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager,
        "Title:   ${title}\n"
        "Artist:  ${artist}\n"
        "Album:   ${album}\n"
        "Котёнок: ${polycat} =D");
    title->XAlign = Text::ContentAlign::Start;
    title->YAlign = Text::ContentAlign::Start;

    std::shared_ptr<Image> cover =
        hBox->AddChild<Image>(Vec2(Size(25, Unit::Percent), 6), Vec2(0, 0),
            Vec2(0, 0), false, true, variableManager, ueberzug, "cover");

    std::shared_ptr<Box> vBox = box.AddChild<Box>(
        Vec2(Size(100, Unit::Percent), Size(25, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager);
    vBox->Orientation = Box::Orientation::Vertical;

    std::shared_ptr<Scale> album = vBox->AddChild<Scale>(
        Vec2(Size(100, Unit::Percent), Size(2, Unit::Pixel)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager, "", "position");
    album->Type = Scale::Type::Horizontal;

    variableManager.AddVariable(
        "title", "playerctl metadata title", Variable::Type::Poll, 0.5);
    variableManager.AddVariable("position",
        "python ~/.config/eww/scripts/mediaplayer.py get_time",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("artist", "playerctl -p ncspot metadata artist",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("album", "playerctl -p ncspot metadata album",
        Variable::Type::Poll, 0.5);
    variableManager.AddVariable("cover",
        "playerctl -p ncspot metadata mpris:artUrl", Variable::Type::Poll, 0.5);
    variableManager.AddVariable("polycat", "polycat", Variable::Type::Stdin);

    struct winsize termSize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &termSize);
    box.UpdateSize(Vec2(0, 0), Vec2(termSize.ws_col, termSize.ws_row), true);

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

        std::this_thread::sleep_until(point);
    }

    return 0;

    (void)argc;
    (void)argv;
}
