#include "Ueberzug.h"
#include "Widget.h"

#include <curl/curl.h>
#include <curl/easy.h>

#include <csignal>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <arpa/inet.h>

#define IMAGE_PATH "/tmp/oneplayer-image-cache"

namespace OnePlayer
{

    void Ueberzug::SpawnDaemon()
    {
        _daemonPipe = popen("ueberzug layer -s", "w");
    }

    Ueberzug::Ueberzug() { SpawnDaemon(); }

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

    void Ueberzug::AddImage(const std::string& path, Vec2 pos, Vec2 maxSize)
    {
        std::stringstream additionCommand;
        additionCommand << "{\"action\":\"add\",\"identifier\":\"" << path
                        << "\", \"max_width\":" << maxSize.x.value
                        << ", \"max_height\":" << maxSize.y.value
                        << ",\"path\":\"" << path << "\", \"x\":" << pos.x.value
                        << ", \"y\":" << pos.y.value << "}";
        std::fprintf(_daemonPipe, "%s\n", additionCommand.str().data());
        fflush(_daemonPipe);
    };

    void Ueberzug::RemoveImage(const std::string& path)
    {
        std::stringstream deletionCommand;
        deletionCommand << "\"action\":\"remove\",\"identifier\":\"" << path
                        << "\"}";
        std::fprintf(_daemonPipe, "%s\n", deletionCommand.str().data());
        fflush(_daemonPipe);
    }

};
