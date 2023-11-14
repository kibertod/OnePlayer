#include "Variable.h"

#include "Widget.h"

#include <chrono>
#include <cstdio>
#include <iostream>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <errno.h>
#include <string.h>
#include <utility>

namespace OnePlayer
{

    CommandExecutionException::CommandExecutionException(
        const std::string& message) noexcept :
        _message(message) {};

    const char* CommandExecutionException::what() const noexcept
    {
        return _message.c_str();
    };

    static void trim(std::string& s)
    {
        s.erase(
            s.begin(), std::find_if(s.begin(), s.end(),
                           [](unsigned char ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(),
                    [](unsigned char ch) { return !std::isspace(ch); })
                    .base(),
            s.end());
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

        trim(result);

        return result;
    }

    Variable::Variable(const std::string& name, const std::string& command,
        enum Type type, VariableManager& manager, float delay) :
        Delay(delay),
        Command(command),
        Name(name),
        Type(type),
        _value("-"),
        _threadRunning(true),
        _manager(manager) {};

    std::string Variable::Value() const
    {
        std::lock_guard guard(_valueMut);
        return _value;
    };

    void Variable::Value(const std::string& value)
    {
        std::lock_guard guard(_valueMut);
        _value = value;
    }

    bool Variable::IsRunning() const
    {
        std::lock_guard guard(_threadRunningMut);
        return _threadRunning;
    }

    void Variable::SetRunning(bool value)
    {
        std::lock_guard guard(_threadRunningMut);
        _threadRunning = value;
    }

    Variable::~Variable() { }

    void Variable::StartUpdateLoop()
    {
        enum Variable::Type type = Type;

        switch (type)
        {
        case (Variable::Type::Poll):
            while (IsRunning())
            {
                auto point = std::chrono::steady_clock::now();

                point +=
                    std::chrono::milliseconds(static_cast<int>(Delay * 1000));

                Value(getCommandOutput(Command));
                _manager.NotifyUpdate();

                std::this_thread::sleep_until(point);
            }
            break;
        case (Variable::Type::Stdin):
            FILE* pipe = popen(Command.c_str(), "r");
            while (IsRunning())
            {
                char buf[2000];
                while (fgets(buf, 2000, pipe) == nullptr && IsRunning())
                    ;

                std::string res(buf);
                trim(res);
                Value(res);
                _manager.NotifyUpdate();
            }
            pclose(pipe);
            break;
        }
    }

    VariableManager::VariableManager() {};

    void VariableManager::AddVariable(const std::string& name,
        const std::string& command, enum Variable::Type type, float delay)
    {
        std::lock_guard guard(_variablesMutex);

        _variables.emplace(std::piecewise_construct, std::tuple(name),
            std::tuple(name, command, type, std::ref(*this), delay));

        _threads.insert({ std::string(name),
            std::thread([](Variable& variable) { variable.StartUpdateLoop(); },
                std::ref(_variables.at(name))) });
    }

    void VariableManager::RemoveVariable(const std::string& name)
    {
        {
            std::lock_guard guard(_variablesMutex);
            _variables.at(name).SetRunning(false);
        }

        _threads.at(name).join();
        _threads.erase(name);

        std::lock_guard guard(_variablesMutex);
        _variables.erase(name);
    }

    std::string VariableManager::GetValue(const std::string& key)
    {
        try
        {
            return _variables.at(key).Value();
        }
        catch (std::out_of_range)
        {
            return "-";
        }
    }

    std::vector<std::string> VariableManager::GetKeys()
    {
        std::vector<std::string> keys;

        {
            std::lock_guard guard(_variablesMutex);
            keys.reserve(_variables.size());

            for (const auto& pair : _variables)
            {
                keys.push_back(pair.first);
            }
        }

        return keys;
    }

    void VariableManager::NotifyUpdate()
    {
        std::lock_guard guard(_updateMut);
        _hasUpdate = true;
    }

    bool VariableManager::HasUpdate()
    {
        std::lock_guard guard(_updateMut);
        bool val = _hasUpdate;
        _hasUpdate = false;
        return val;
    }

}
