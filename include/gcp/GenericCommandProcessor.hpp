#pragma once

#include <functional>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "gcp/index_sequence.hpp"

namespace gcp {

class GenericCommandProcessor final
{
public: // methods
    template <class F>
    void registerCommand(const std::string &name, F &&f)
    {
        RegisterCommandHelper<F>{}(*this, name, std::forward<F>(f));
    }

    void process(const std::string &command)
    {
        ///@todo Implement proper tokenization

        std::istringstream stream(command);

        std::vector<std::string> words{
            std::istream_iterator<std::string>(stream),
            std::istream_iterator<std::string>()};

        if (words.empty())
        {
            return;
        }

        if (m_handlers.find(words[0]) == std::end(m_handlers))
        {
            if (m_error_callback)
            {
                m_error_callback("unknown command '" + words[0] + "'");
            }
            return;
        }

        std::vector<std::string> args{++std::begin(words), std::end(words)};

        m_handlers[words[0]](args);
    }

    void registerErrorCallback(
        const std::function<void(const std::string &)> &callback)
    {
        m_error_callback = callback;
    }

private: // types
    using HandlerType = std::function<void(const std::vector<std::string> &)>;

    template <class T>
    struct RegisterCommandHelper
        : public RegisterCommandHelper<decltype(&T::operator())>
    {
    };

    template <class C, class R, class... Args>
    struct RegisterCommandHelper<R (C::*)(Args...) const>
    {
        void operator()(
            GenericCommandProcessor &processor,
            const std::string &name,
            const std::function<void(Args...)> &handler) const
        {
            using IndexSequence = make_index_sequence<sizeof...(Args)>;
            processor.registerCommandImpl(name, handler, IndexSequence{});
        }
    };

private: // methods
    template <class... Args, std::size_t... Indices>
    void registerCommandImpl(
        const std::string &name,
        const std::function<void(Args...)> &handler,
        index_sequence<Indices...>)
    {
        m_handlers[name] = [=](const std::vector<std::string> &args) {
            if (args.size() != sizeof...(Args))
            {
                if (m_error_callback)
                {
                    m_error_callback("invalid number of arguments");
                }
                return;
            }

            try
            {
                handler(convert<Args>(args[Indices])...);
            }
            catch (const std::exception &e)
            {
                if (m_error_callback)
                {
                    m_error_callback(e.what());
                }
            }
        };
    }

    template <class T>
    T convert(const std::string &str);

private: // fields
    std::map<std::string, HandlerType> m_handlers;
    std::function<void(const std::string &)> m_error_callback;
};

template <>
inline int GenericCommandProcessor::convert(const std::string &str)
{
    return std::stoi(str);
}

template <>
inline long GenericCommandProcessor::convert(const std::string &str)
{
    return std::stol(str);
}

template <>
inline long long GenericCommandProcessor::convert(const std::string &str)
{
    return std::stoll(str);
}

template <>
inline unsigned long GenericCommandProcessor::convert(const std::string &str)
{
    return std::stoul(str);
}

template <>
inline unsigned long long GenericCommandProcessor::convert(
    const std::string &str)
{
    return std::stoull(str);
}

template <>
inline float GenericCommandProcessor::convert(const std::string &str)
{
    return std::stof(str);
}

template <>
inline double GenericCommandProcessor::convert(const std::string &str)
{
    return std::stod(str);
}

template <>
inline long double GenericCommandProcessor::convert(const std::string &str)
{
    return std::stold(str);
}

template <>
inline std::string GenericCommandProcessor::convert(const std::string &str)
{
    return str;
}

template <>
inline const std::string &GenericCommandProcessor::convert(
    const std::string &str)
{
    return str;
}

template <>
inline char GenericCommandProcessor::convert(const std::string &str)
{
    return str[0];
}

} // namespace gcp
