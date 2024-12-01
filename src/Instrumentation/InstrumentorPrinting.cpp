#include "Instrumentation/InstrumentorPrinting.hpp"
#include <shared_mutex>

class comma_numpunct : public std::numpunct<char>
{
  protected:
    virtual char do_thousands_sep() const override
    {
        return ',';
    }

    virtual std::string do_grouping() const override
    {
        return "\03";
    }
};

void InstrumentorPrinting::PrintSessionData(const std::unordered_map<std::string, InstrumentTime> &functionTimes,
                                            int64_t totalRuntime)
{
    if (functionTimes.empty())
    {
        return;
    }

    int maxNameLength = 0;
    uint64_t totalCount = 0;

    for (const auto &pair : functionTimes)
    {
        totalCount += pair.second.Count;
        maxNameLength = std::max(maxNameLength, static_cast<int>(pair.first.length()));
    }

    std::vector<std::pair<std::string, InstrumentTime>> sortedFunctionTimes(functionTimes.begin(), functionTimes.end());
    std::sort(sortedFunctionTimes.begin(), sortedFunctionTimes.end(),
              [](const std::pair<std::string, InstrumentTime> &a, const std::pair<std::string, InstrumentTime> &b) {
                  return a.second.getAverageTime() > b.second.getAverageTime();
              });

    std::string horizontalLine = "+" + std::string(maxNameLength + 2, '-') + "+" + std::string(17, '-') + "+" +
                                 std::string(14, '-') + "+" + std::string(14, '-') + "+";

    std::cout << "\n" << horizontalLine << "\n";
    std::cout << "| " << std::setw(maxNameLength) << std::left << "Function Name" << " | " << std::setw(15) << "Count"
              << " | " << std::setw(12) << "Total Time" << " | " << std::setw(12) << "Average Time" << " |\n";
    std::cout << horizontalLine << "\n";

    std::locale comma_locale(std::locale(), new comma_numpunct());
    std::cout.imbue(comma_locale);
    for (const auto &pair : sortedFunctionTimes)
    {
        std::cout << "| " << std::setw(maxNameLength) << std::left << pair.first << " | " << std::setw(15)
                  << pair.second.Count << " | "
                  << pair.second.getConvertedTime(12, static_cast<double>(pair.second.TotalTime)) << " | "
                  << pair.second.getConvertedTime(12, pair.second.getAverageTime()) << " |\n";
    }

    std::cout << horizontalLine << "\n";
    InstrumentTime totalInstrumentTime;

    std::cout << "| " << std::setw(maxNameLength) << std::left << "Total / Runtime" << " | " << std::setw(15)
              << totalCount << " | " << std::setw(12)
              << totalInstrumentTime.getConvertedTime(12, static_cast<double>(totalRuntime)) << " | " << std::setw(12)
              << "N/A" << " |\n";

    std::cout << horizontalLine << "\n";
}
