#include "Instrumentation/InstrumentorCore.hpp"
#include "Instrumentation/InstrumentorIO.hpp"

Instrumentor::Instrumentor() : m_CurrentSession(nullptr), m_ProfileCount(0)
{
}

Instrumentor::~Instrumentor()
{
    EndSession();
}

void Instrumentor::BeginSession(const std::string &name, const std::string &filepath, const bool isWritingProfiles)
{
    if (isWritingProfiles)
    {
        InstrumentorIO::OpenFile(filepath);
    }
    m_CurrentSession = std::make_unique<InstrumentationSession>();
    m_CurrentSession->StartTime =
        std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())
            .time_since_epoch()
            .count();
    m_CurrentSession->Name = name;
    m_CurrentSession->isWritingProfiles = isWritingProfiles;
    if (isWritingProfiles)
    {
        InstrumentorIO::WriteHeader(m_CurrentSession->Name);
    }
}

void Instrumentor::EndSession()
{
    if (m_CurrentSession && m_CurrentSession->isWritingProfiles)
    {
        InstrumentorIO::WriteCacheToFile(m_ProfileCache);
        InstrumentorIO::WriteFooter(m_FunctionTimes, m_ProfileCount);
        InstrumentorIO::CloseFile();
    }
    m_CurrentSession = nullptr;
    m_ProfileCount = 0;
}

void Instrumentor::WriteProfile(const ProfileResult &result)
{
    if (m_CurrentSession && m_CurrentSession->isWritingProfiles)
    {
        AddProfileToCache(result);
        if (m_ProfileCache.size() >= 10000)
        {
            std::lock_guard<std::shared_mutex> lock(m_SharedMutex);
            InstrumentorIO::WriteCacheToFile(m_ProfileCache);
            m_ProfileCache.clear();
        }
    }
}

void Instrumentor::AddFunctionTime(const std::string &name, long long time)
{
    std::lock_guard<std::shared_mutex> lock(m_SharedMutex);

    auto &functionData = m_FunctionTimes[name];

    if (!functionData.Initialized)
    {
        functionData.Start = std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())
                                 .time_since_epoch()
                                 .count();
        functionData.Initialized = true;
    }

    functionData.TotalTime += time;
    functionData.Count++;
}

Instrumentor &Instrumentor::Get()
{
    static Instrumentor instance;
    return instance;
}

void Instrumentor::AddProfileToCache(const ProfileResult &result)
{
    m_ProfileCache.push_back(result);
}

int64_t Instrumentor::GetRuntime() const
{
    return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now())
               .time_since_epoch()
               .count() -
           m_CurrentSession->StartTime;
}