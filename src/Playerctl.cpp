#include "Playerctl.h"

#include <string>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>

namespace OnePlayer::Playerctl
{

    CommandExecutionException::CommandExecutionException(
        const std::string& message) noexcept :
        _message(message) {};

    const char* CommandExecutionException::what() const noexcept
    {
        return _message.c_str();
    };

    static void ltrim(std::string& s)
    {
        s.erase(
            s.begin(), std::find_if(s.begin(), s.end(),
                           [](unsigned char ch) { return !std::isspace(ch); }));
    }

    static void rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(),
                    [](unsigned char ch) { return !std::isspace(ch); })
                    .base(),
            s.end());
    }

    static void trim(std::string& s)
    {
        rtrim(s);
        ltrim(s);
    }

    std::string getCommandOutput(const std::string& command)
    {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen(command.c_str(), "r"), pclose);
        if (!pipe)
        {
            throw CommandExecutionException("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        {
            result += buffer.data();
        }

        rtrim(result);

        return result;
    }

    std::string getCheckedValue(const std::string& value)
    {
        std::string res(value);

        trim(res);

        if (res == "No player could handle this command")
            return "";

        return res;
    }

    std::string getCheckedProperty(
        const std::string& metadata, const std::string& property)
    {
        try
        {
            std::string res = metadata.substr(metadata.find(property));
            res = res.substr(res.find(":") + 1);
            res = res.substr(0, res.find("|"));
            return getCheckedValue(res + "\n");
        }
        catch (std::out_of_range _)
        {
            return "";
        }
    }

    MetaData::MetaData(const std::string& player)
    {
        std::stringstream commandPrefix;
        commandPrefix << "playerctl ";

        if (player != "")
            commandPrefix << "-p " << player << " ";

        std::string metaDataOut = getCommandOutput(
            commandPrefix.str() +
            "-f \"title:{{ title "
            "}}|artist:{{artist}}|album:{{album}}|artUrl:"
            "{{mpris:artUrl}}|length:{{mpris:length}}\" metadata");
        std::string positionOut =
            getCommandOutput(commandPrefix.str() + "position");
        std::string statusOut =
            getCommandOutput(commandPrefix.str() + "status");

        Title = getCheckedProperty(metaDataOut, "title");
        Artist = getCheckedProperty(metaDataOut, "artist");
        Album = getCheckedProperty(metaDataOut, "album");
        ImageUrl = getCheckedProperty(metaDataOut, "artUrl");
        Length = getCheckedProperty(metaDataOut, "length");
        Position = getCheckedValue(positionOut);
        Status = getCheckedValue(statusOut);
    };

    void playNext(const std::string& player)
    {
        std::stringstream commandStream;
        commandStream << "playerctl ";

        if (player != "")
            commandStream << "-p " << player << " ";

        commandStream << "next";

        getCommandOutput(commandStream.str());
    }

    void playPrevious(const std::string& player)
    {
        std::stringstream commandStream;
        commandStream << "playerctl ";

        if (player != "")
            commandStream << "-p " << player << " ";

        commandStream << "previous";

        getCommandOutput(commandStream.str());
    }

    void togglePlayPause(const std::string& player)
    {
        std::stringstream commandStream;
        commandStream << "playerctl ";

        if (player != "")
            commandStream << "-p " << player << " ";

        commandStream << "play-pause";

        getCommandOutput(commandStream.str());
    }

}
