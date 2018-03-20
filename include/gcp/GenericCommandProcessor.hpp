#pragma once

#include <algorithm>
#include <cstddef>
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

    void process(const std::string &str)
    {
        std::vector<std::string> words;
        std::size_t pos = 0;

        auto skip_spaces = [&]() {
            while (std::isspace(str[pos]) && pos < str.size())
            {
                pos++;
            }
        };

        while (pos < str.size())
        {
            if (std::isspace(str[pos]))
            {
                skip_spaces();
            }
            else if (str[pos] == '(')
            {
                words.emplace_back("");

                int depth = 0;

                while (pos < str.size())
                {
                    if (str[pos] == ')')
                    {
                        if (--depth == 0)
                        {
                            pos++;
                            continue;
                        }
                    }
                    else
                    {
                        if (str[pos] == '(')
                        {
                            if (depth++ == 0)
                            {
                                pos++;
                                continue;
                            }
                        }
                    }

                    if (depth)
                    {
                        if (std::isspace(str[pos]))
                        {
                            words.back() += str[pos++];
                            skip_spaces();
                        }
                        else
                        {
                            words.back() += str[pos++];
                        }
                    }
                    else
                    {
                        break;
                    }
                }

                if (depth)
                {
                    if (m_error_callback)
                    {
                        m_error_callback("unmatched parentheses");
                    }
                    return;
                }

                words.back().erase(
                    words.back().begin(),
                    std::find_if(
                        words.back().begin(), words.back().end(), [](char c) {
                            return !std::isspace(c);
                        }));

                words.back().erase(
                    std::find_if(
                        words.back().rbegin(),
                        words.back().rend(),
                        [](char c) { return !std::isspace(c); })
                        .base(),
                    words.back().end());
            }
            else if (str[pos] == '"')
            {
                words.emplace_back("");

                pos++;

                while (pos < str.size() && str[pos] != '"')
                {
                    if (str[pos] == '\\')
                    {
                        if (++pos >= str.size())
                        {
                            if (m_error_callback)
                            {
                                m_error_callback("invalid escape sequence");
                            }
                            return;
                        }
                    }

                    words.back() += str[pos++];
                }

                if (str[pos++] != '"')
                {
                    if (m_error_callback)
                    {
                        m_error_callback("invalid string constant");
                    }
                    return;
                }
            }
            else if (str[pos] == ')')
            {
                if (m_error_callback)
                {
                    m_error_callback("unmatched parentheses");
                }
                return;
            }
            else
            {
                words.emplace_back("");

                while (str[pos] != '(' && str[pos] != ')' &&
                       !std::isspace(str[pos]) && pos < str.size())
                {
                    words.back() += str[pos++];
                }
            }
        }

        if (words.empty())
        {
            return;
        }

        std::vector<std::string> args{++std::begin(words), std::end(words)};

        const auto name_it = m_handlers.find(words[0]);

        if (name_it == std::end(m_handlers))
        {
            if (m_error_callback)
            {
                m_error_callback("unknown command '" + words[0] + "'");
            }
            return;
        }

        const auto args_it = name_it->second.find(args.size());

        if (args_it == std::end(name_it->second))
        {
            if (m_error_callback)
            {
                std::vector<std::size_t> arg_counts;
                for (const auto &it : name_it->second)
                {
                    arg_counts.emplace_back(it.first);
                }

                std::string arg_counts_str;

                if (arg_counts.size() == 1)
                {
                    arg_counts_str = std::to_string(arg_counts[0]);
                }
                else
                {
                    arg_counts_str += "[";

                    for (std::size_t i = 0; i < arg_counts.size(); i++)
                    {
                        if (i > 0)
                        {
                            arg_counts_str += "|";
                        }

                        arg_counts_str += std::to_string(arg_counts[i]);
                    }

                    arg_counts_str += "]";
                }

                m_error_callback(
                    "invalid number of arguments (" +
                    std::to_string(args.size()) + "/" + arg_counts_str + ")");
            }
            return;
        }

        args_it->second(args);
    }

    void registerErrorCallback(
        const std::function<void(const std::string &)> &callback)
    {
        m_error_callback = callback;
    }

private: // types
    template <class T>
    using Decay = typename std::decay<T>::type;

    using HandlerType = std::function<void(const std::vector<std::string> &)>;

    template <class F>
    struct RegisterCommandHelper
        : public RegisterCommandHelper<decltype(&F::operator())>
    {
    };

    template <class R, class... Args>
    struct RegisterCommandHelper<R (*)(Args...)>
        : public RegisterCommandHelper<R(Args...)>
    {
    };

    template <class C, class R, class... Args>
    struct RegisterCommandHelper<R (C::*)(Args...) const>
        : public RegisterCommandHelper<R(Args...)>
    {
    };

    template <class R, class... Args>
    struct RegisterCommandHelper<R(Args...)>
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
        m_handlers[name][sizeof...(Args)] =
            [=](const std::vector<std::string> &args) {
                try
                {
                    handler(convert<Decay<Args>>(args[Indices])...);
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
    std::map<std::string, std::map<std::size_t, HandlerType>> m_handlers;
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
inline char GenericCommandProcessor::convert(const std::string &str)
{
    return str[0];
}

} // namespace gcp
