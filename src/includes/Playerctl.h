#pragma once

#include <string>
#include <exception>

namespace OnePlayer::Playerctl
{

    class CommandExecutionException : public std::exception
    {
    public:
        CommandExecutionException(const std::string& message) noexcept;
        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    struct MetaData
    {
        std::string Title;
        std::string Artist;
        std::string Album;
        std::string ImageUrl;
        std::string Length;
        std::string Position;
        std::string Status;

        MetaData(const std::string& player);
    };

    void playNext(const std::string& player = "");

    void playPrevious(const std::string& player = "");

    void togglePlayPause(const std::string& player = "");

}
