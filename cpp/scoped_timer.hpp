#pragma once

#include <chrono>

class ScopedTimer
{
public:
    inline std::int64_t elapsed_ns() const
    {
        return (std::chrono::high_resolution_clock::now() - begin_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point begin_{std::chrono::high_resolution_clock::now()};
};
