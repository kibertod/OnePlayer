#include <sys/ioctl.h>
#include <chrono>
#include <cstdio>
#include <curses.h>
#include <thread>
#include <iostream>

#include "Variable.h"
#include "Widget.h"

int main(int argc, char** argv)
{
    using namespace OnePlayer;
    setlocale(LC_ALL, "");

    VariableManager variableManager;

    initscr();
    curs_set(0);

    Box box(Vec2(20, 20), Vec2(0, 0), Vec2(0, 0), false, true, variableManager);
    std::shared_ptr<Text> title = box.AddChild<Text>(
        Vec2(Size(50, Unit::Percent), Size(100, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), true, true, variableManager,
        "${polycat}${polycat}Title\n${polycat}${polycat}${title}\n${polycat}$"
        "{polycat}котёнок");
    title->XAlign = Text::ContentAlign::Center;
    title->YAlign = Text::ContentAlign::Center;

    std::shared_ptr<Box> vBox = box.AddChild<Box>(
        Vec2(Size(50, Unit::Percent), Size(100, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), false, true, variableManager);
    vBox->Orientation = Box::Orientation::Vertical;

    std::shared_ptr<Text> artist = vBox->AddChild<Text>(
        Vec2(Size(100, Unit::Percent), Size(50, Unit::Percent)), Vec2(0, 0),
        Vec2(0, 0), true, true, variableManager,
        "${polycat}${polycat}Artist\n${polycat}${polycat}${artist}\n${"
        "polycat}${polycat}котёнок");
    artist->XAlign = Text::ContentAlign::Center;
    artist->YAlign = Text::ContentAlign::Center;

    std::shared_ptr<Text> album =
        vBox->AddChild<Text>(Vec2(Size(100, Unit::Percent), 10), Vec2(0, 0),
            Vec2(0, 0), true, true, variableManager,
            "${polycat}${polycat}Album\n${polycat}${polycat}${album}\n${"
            "polycat}$"
            "{polycat}котёнок");
    album->XAlign = Text::ContentAlign::Center;
    album->YAlign = Text::ContentAlign::Center;

    variableManager.AddVariable(
        "title", "playerctl metadata title", Variable::Type::Poll, 0.5);
    variableManager.AddVariable(
        "artist", "playerctl metadata artist", Variable::Type::Poll, 0.5);
    variableManager.AddVariable(
        "album", "playerctl metadata album", Variable::Type::Poll, 0.5);
    variableManager.AddVariable("polycat", "polycat", Variable::Type::Stdin);

    box.Orientation = Box::Orientation::Horizontal;

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
