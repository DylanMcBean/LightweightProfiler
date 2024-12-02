#include "Profiler.hpp"
#include <locale>
#include <sstream>

void InstrumentTime::addTiming(int64_t time) {
    Timings.push_back(time);
    TotalTime += time;
    Count++;
}

double InstrumentTime::getAverageTime() const {
    return Count == 0 ? 0 : static_cast<double>(TotalTime) / Count;
}

std::vector<double> InstrumentTime::getPercentiles() const {
    if (Timings.empty()) return {};

    std::vector<int64_t> sortedTimings = Timings;
    std::sort(sortedTimings.begin(), sortedTimings.end());
    size_t n = sortedTimings.size();

    auto interpolate = [&](double index) -> double {
        size_t lowIndex = static_cast<size_t>(index);
        double fraction = index - lowIndex;

        if (lowIndex + 1 < n) {
            return sortedTimings[lowIndex] * (1.0 - fraction) + sortedTimings[lowIndex + 1] * fraction;
        } else {
            return sortedTimings[lowIndex];
        }
    };

    return {
        interpolate(n * 5 / 100.0),
        interpolate(n * 50 / 100.0),
        interpolate(n * 95 / 100.0)
    };
}



std::string InstrumentTime::getConvertedTime(int stringLength, double timeToConvert) const {
    constexpr std::array<const char*, 7> timeUnits = {" ns", " us", " ms", "sec", "min", " hr", "day"};
    constexpr std::array<int, 6> timeUnitValues = {1000, 1000, 1000, 60, 60, 24};

    size_t unitIndex = 0;
    while (unitIndex < timeUnitValues.size() && timeToConvert >= timeUnitValues[unitIndex]) {
        timeToConvert /= timeUnitValues[unitIndex];
        ++unitIndex;
    }

#if __cplusplus > 201703L // check for c++20 support
    return std::format("{:<{}.2f}{}", timeToConvert, stringLength - 3, timeUnits[unitIndex]);
#else // fallback to stringstream
	std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << std::left << std::setw(static_cast<std::streamsize>(stringLength) - 3) << timeToConvert << timeUnits[unitIndex];
	return ss.str();
#endif
}

Instrumentor& Instrumentor::Get() {
    static Instrumentor instance;
    return instance;
}

void Instrumentor::AddFunctionTime(const std::string& name, int64_t time) {
    std::lock_guard<std::shared_mutex> lock(m_SharedMutex);
    auto& functionData = m_FunctionTimes[name];

    if (!functionData.Initialized) {
        functionData.Initialized = true;
    }

    functionData.addTiming(time);
}

void Instrumentor::PrintResults() {
    if (m_FunctionTimes.empty()) return;

    std::vector<std::pair<std::string, InstrumentTime>> sortedFunctionTimes(m_FunctionTimes.begin(), m_FunctionTimes.end());

    // Sort by 95th percentile
    std::sort(sortedFunctionTimes.begin(), sortedFunctionTimes.end(),
              [](const auto& a, const auto& b) {
                  auto aPercentiles = a.second.getPercentiles();
                  auto bPercentiles = b.second.getPercentiles();
                  double a95 = (aPercentiles.size() > 2) ? aPercentiles[2] : 0.0;
                  double b95 = (bPercentiles.size() > 2) ? bPercentiles[2] : 0.0;
                  return a95 > b95; // Descending order
              });

	int maxNameLength = 13; // "Function Name"
    for (const auto& pair : sortedFunctionTimes) {
        maxNameLength = std::max(maxNameLength, static_cast<int>(pair.first.length()));
    }

    const std::string horizontalLine = "+" + std::string(maxNameLength + 2, '-') + "+" +
                                       std::string(10, '-') + "+" + std::string(15, '-') + "+" +
                                       std::string(12, '-') + "+" + std::string(12, '-') + "+" +
                                       std::string(12, '-') + "+";

    std::cout << horizontalLine << "\n";
    std::cout << "| " << std::setw(maxNameLength) << std::left << "Function Name"
              << " | " << std::setw(8) << "Count"
              << " | " << std::setw(13) << "Total Time"
              << " | " << std::setw(10) << "5%"
              << " | " << std::setw(10) << "50%"
              << " | " << std::setw(10) << "95%" << " |\n";
    std::cout << horizontalLine << "\n";

    for (const auto& pair : sortedFunctionTimes) {
        const auto& functionData = pair.second;
        auto percentiles = functionData.getPercentiles();

        std::cout << "| " << std::setw(maxNameLength) << std::left << pair.first
                  << " | " << std::setw(8) << functionData.Count
                  << " | " << std::setw(13) << functionData.getConvertedTime(13, static_cast<double>(functionData.TotalTime))
                  << " | " << std::setw(10) << (percentiles.size() > 0 ? functionData.getConvertedTime(10, percentiles[0]) : "N/A")
                  << " | " << std::setw(10) << (percentiles.size() > 1 ? functionData.getConvertedTime(10, percentiles[1]) : "N/A")
                  << " | " << std::setw(10) << (percentiles.size() > 2 ? functionData.getConvertedTime(10, percentiles[2]) : "N/A")
                  << " |\n";
    }

    std::cout << horizontalLine << "\n";
}

Instrumentor::~Instrumentor() {
    PrintResults();
}

InstrumentationTimer::InstrumentationTimer(const char* name) : m_Name(name), m_Stopped(false) {
    m_StartTimepoint = std::chrono::steady_clock::now();
}

InstrumentationTimer::~InstrumentationTimer() {
    if (!m_Stopped) Stop();
}

void InstrumentationTimer::Stop() {
    auto endTimepoint = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTimepoint - m_StartTimepoint).count();
    Instrumentor::Get().AddFunctionTime(m_Name, duration);
    m_Stopped = true;
}
