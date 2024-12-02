#pragma once

#include <chrono>
#include <unordered_map>
#include <shared_mutex>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <format>

struct InstrumentTime {
    int64_t TotalTime = 0;
    uint64_t Count = 0;
    bool Initialized = false;
    std::vector<int64_t> Timings;

    void addTiming(int64_t time);
    double getAverageTime() const;
    std::vector<double> getPercentiles() const;
    std::string getConvertedTime(int stringLength, double timeToConvert) const;
};

class Instrumentor {
public:
    static Instrumentor& Get();

    void AddFunctionTime(const std::string& name, int64_t time);
    ~Instrumentor();

private:
    std::unordered_map<std::string, InstrumentTime> m_FunctionTimes;
    std::shared_mutex m_SharedMutex;

    Instrumentor() = default;
    void PrintResults();
};

class InstrumentationTimer {
public:
    explicit InstrumentationTimer(const char* name);
    ~InstrumentationTimer();
    void Stop();

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::steady_clock> m_StartTimepoint;
    bool m_Stopped;
};

#if PROFILING
#ifdef _MSC_VER // Microsoft Compiler
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__FUNCTION__)
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#else // Clang, GCC
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__PRETTY_FUNCTION__)
#define PROFILE_SCOPE(name) InstrumentationTimer timer##__LINE__(name)
#endif
#else
#define PROFILE_FUNCTION()
#define PROFILE_SCOPE(name)
#endif
