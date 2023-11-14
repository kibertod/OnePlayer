#pragma once

#include "Widget.fwd.h"
#include "Variable.fwd.h"

#include <unordered_map>
#include <mutex>
#include <thread>
#include <vector>
#include <string>

namespace OnePlayer
{

    class CommandExecutionException : public std::exception
    {
    public:
        CommandExecutionException(const std::string& message) noexcept;
        const char* what() const noexcept override;

    private:
        std::string _message;
    };

    class Variable
    {

    public:
        enum class Type
        {
            Poll,
            Stdin
        };

        const float Delay;
        const std::string Command;
        const std::string Name;
        const Type Type;

        std::string Value() const;
        void Value(const std::string& value);
        bool IsRunning() const;
        void SetRunning(bool value);

        Variable(const std::string& name, const std::string& command,
            enum Type type, VariableManager& manager, float delay = 0);
        ~Variable();
        void StartUpdateLoop();

    private:
        mutable std::mutex _valueMut;
        mutable std::mutex _threadRunningMut;
        std::string _value;
        bool _threadRunning;
        VariableManager& _manager;
    };

    class VariableManager
    {
    public:
        VariableManager();

        void AddVariable(const std::string& name, const std::string& command,
            enum Variable::Type type, float delay = 0);
        void RemoveVariable(const std::string& key);
        std::vector<std::string> GetKeys();
        std::string GetValue(const std::string& key);
        void NotifyUpdate();
        bool HasUpdate();

    private:
        bool _hasUpdate;
        std::mutex _updateMut;
        std::unordered_map<std::string, Variable> _variables;
        std::unordered_map<std::string, std::thread> _threads;
        std::mutex _variablesMutex;
    };

}
