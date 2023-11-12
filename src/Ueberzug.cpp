#include "Ueberzug.h"

#include "ScreenManager.fwd.h"

#include <algorithm>
#include <curl/curl.h>
#include <curl/easy.h>

#include <csignal>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#define IMAGE_PATH "/tmp/oneplayer-image-cache"

namespace OnePlayer::Ueberzug
{

    void Ueberzug::SpawnDaemon()
    {
        _daemonPipe = popen("ueberzug layer -s", "w");
    }

    Ueberzug::Ueberzug() :
        _lastUrl(""),
        _lastPos(0, 0)
    {
        SpawnDaemon();
    }

    Ueberzug::~Ueberzug() { pclose(_daemonPipe); }

    size_t write_data(void* ptr, size_t size, size_t nmemb, FILE* stream)
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    std::string Ueberzug::CacheImage(const std::string& url)
    {
        FILE* fp = std::fopen(IMAGE_PATH, "wb");
        CURL* curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, url.data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);

        curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        fclose(fp);

        return IMAGE_PATH;
    };

    void Ueberzug::ChangeImage(
        const std::string& url, const XYPair& pos, const XYPair& maxSize)
    {
        if (_lastUrl == url && _lastPos == pos)
            return;

        std::string path = CacheImage(url);

        if (_lastUrl != "")
        {
            std::stringstream deletionCommand;
            deletionCommand << "\"action\":\"remove\",\"identifier\":\""
                            << _lastUrl << "\"}";
            std::fprintf(_daemonPipe, "%s\n", deletionCommand.str().data());
            fflush(_daemonPipe);
        }

        std::stringstream additionCommand;

        additionCommand << "{\"action\":\"add\",\"identifier\":\"" << url
                        << "\", \"max_width\":" << maxSize.x
                        << ", \"max_height\":" << maxSize.y << ",\"path\":\""
                        << path << "\", \"x\":" << pos.x << ", \"y\":" << pos.y
                        << "}";
        std::fprintf(_daemonPipe, "%s\n", additionCommand.str().data());
        fflush(_daemonPipe);

        _lastUrl = url;
        _lastPos = pos;
    };

};
