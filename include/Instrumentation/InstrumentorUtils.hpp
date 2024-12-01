#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <iomanip>
#include <locale>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

struct ProfileResult
{
    std::string Name;
    int64_t Start, End;
    uint32_t ThreadID;
};

struct InstrumentTime
{
    int64_t Start = 0;
    int64_t TotalTime = 0;
    uint64_t Count = 0;
    bool Initialized = false;

    double getAverageTime() const
    {
        return Count == 0 ? 0 : static_cast<double>(TotalTime) / Count;
    }

    std::string getConvertedTime(int stringLength, double timeToConvert) const
    {
        constexpr std::array<std::string_view, 7> timeUnits = {"ns", "us", "ms", " s", " m", " h", " d"};
        constexpr std::array<int, 6> timeUnitValues = {1000, 1000, 1000, 60, 60, 24};

        size_t unitIndex = 0;
        while (unitIndex < timeUnitValues.size() && timeToConvert >= timeUnitValues[unitIndex])
        {
            timeToConvert /= timeUnitValues[unitIndex];
            ++unitIndex;
        }

        return std::format("{:<{}.2f}{}", timeToConvert, stringLength - 2, timeUnits[unitIndex]);
    }
};

struct InstrumentationSession
{
    std::string Name;
    bool isWritingProfiles = false;
    int64_t StartTime;
};