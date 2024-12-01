#include "Instrumentation/InstrumentationTimer.hpp"
#include "Instrumentation/InstrumentorCore.hpp"

InstrumentationTimer::InstrumentationTimer(const char *name) : m_Name(name), m_Stopped(false)
{
    m_StartTimepoint = std::chrono::steady_clock::now();
}

InstrumentationTimer::~InstrumentationTimer()
{
    if (!m_Stopped)
        Stop();
}

void InstrumentationTimer::Stop()
{
    auto endTimepoint = std::chrono::steady_clock::now();
    std::chrono::nanoseconds duration =
        std::chrono::duration_cast<std::chrono::nanoseconds>(endTimepoint - m_StartTimepoint);

    uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
    auto &instrumentor = Instrumentor::Get();
    if (duration.count() > 0)
    {
        instrumentor.WriteProfile(
            {m_Name, m_StartTimepoint.time_since_epoch().count(), endTimepoint.time_since_epoch().count(), threadID});
    }
    instrumentor.AddFunctionTime(m_Name, duration.count());

    m_Stopped = true;
}