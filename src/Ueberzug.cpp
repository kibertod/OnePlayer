#include "Ueberzug.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

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

    void Ueberzug::ChangeImage(const std::string& url, Vec2 pos, Vec2 maxSize)
    {
        if ((_lastUrl == url && _lastPos == pos) || url == "")
            return;

        std::stringstream deletionCommand;
        deletionCommand << "\"action\":\"remove\",\"identifier\":\"cover\"}";
        std::fprintf(_daemonPipe, "%s\n", deletionCommand.str().data());
        fflush(_daemonPipe);

        std::string path;
        if (_lastUrl != url)
            path = CacheImage(url);
        else
            path = IMAGE_PATH;

        std::stringstream additionCommand;
        additionCommand
            << "{\"action\":\"add\",\"identifier\":\"cover\", \"max_width\":"
            << maxSize.x.value << ", \"max_height\":" << maxSize.y.value
            << ",\"path\":\"" << path << "\", \"x\":" << pos.x.value
            << ", \"y\":" << pos.y.value << "}";
        std::fprintf(_daemonPipe, "%s\n", additionCommand.str().data());
        fflush(_daemonPipe);

        std::ofstream log;
        log.open("penis.lol", std::ios::app);
        log << url << " и ещё короче \"" << _lastUrl << "\"" << std::endl
            << std::endl;
        _lastUrl = url;
        _lastPos = pos;
    };

};
