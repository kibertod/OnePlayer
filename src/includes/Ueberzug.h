#pragma once

#include <string>

#include "ScreenManager.h"

namespace OnePlayer::Ueberzug
{

    class Ueberzug
    {
    public:
        Ueberzug();
        ~Ueberzug();
        void ChangeImage(
            const std::string& url, const XYPair& pos, const XYPair& maxSize);
        std::string CacheImage(const std::string& url);

    private:
        FILE* _daemonPipe;
        std::string _lastUrl;
        XYPair _lastPos;

        void SpawnDaemon();
        void SendData(const std::string& data);
    };

};
