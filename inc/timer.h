#pragma once
#include <chrono>

class timer_t
{
protected:
    std::chrono::high_resolution_clock::time_point begin;
    std::chrono::high_resolution_clock::time_point end;

public:
    timer_t() { begin = end = std::chrono::high_resolution_clock::now(); }

    inline void Start() { begin = std::chrono::high_resolution_clock::now(); }
    inline void Stop() { end = std::chrono::high_resolution_clock::now();    }

    template <typename T>
    long long Get_Result() { return std::chrono::duration_cast<T>(end - begin).count(); }
};