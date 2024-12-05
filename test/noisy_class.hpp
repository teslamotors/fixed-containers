#pragma once

#include <iostream>
#include <string>
#include <string_view>

struct NoisyClass
{
    std::string s;

    NoisyClass()
      : s()
    {
        std::cout << "default constructor" << '\n';
    }
    NoisyClass(const std::string_view sv0)
      : s(" ")
    {
        s.append(sv0);
        std::cout << "constructor" << s << '\n';
    }
    NoisyClass(const NoisyClass& /*unused*/) { std::cout << "copy constructor" << s << '\n'; }
    NoisyClass(NoisyClass&& /*unused*/) noexcept { std::cout << "move constructor" << s << '\n'; }
    NoisyClass& operator=(const NoisyClass& /*unused*/)
    {
        std::cout << "copy assignment operator" << s << '\n';
        return *this;
    }
    NoisyClass& operator=(NoisyClass&& /*unused*/) noexcept
    {
        std::cout << "move assignment operator" << s << '\n';
        return *this;
    }

    ~NoisyClass() { std::cout << "destructor" << s << '\n'; }
};
