#pragma once

#include <string>

#include "Ueberzug.fwd.h"
#include "Widget.fwd.h"

namespace OnePlayer
{

    class Ueberzug
    {
    public:
        Ueberzug();
        ~Ueberzug();
        std::string CacheImage(const std::string& url);
        void AddImage(const std::string& path, Vec2 pos, Vec2 maxSize);
        void RemoveImage(const std::string& url);

    private:
        FILE* _daemonPipe;

        void SpawnDaemon();
        void SendData(const std::string& data);
    };

};
