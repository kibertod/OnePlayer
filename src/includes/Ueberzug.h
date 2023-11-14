#pragma once

#include <string>

#include "Widget.h"

namespace OnePlayer::Ueberzug
{

    class Ueberzug
    {
    public:
        Ueberzug();
        ~Ueberzug();
        void ChangeImage(const std::string& url, Vec2 pos, Vec2 maxSize);

    private:
        FILE* _daemonPipe;
        std::string _lastUrl;
        Vec2 _lastPos;

        std::string CacheImage(const std::string& url);
        void SpawnDaemon();
        void SendData(const std::string& data);
    };

};
