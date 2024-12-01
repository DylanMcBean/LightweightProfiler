#pragma once

#include "Instrumentation/InstrumentorUtils.hpp"
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

class Instrumentor
{
  private:
    std::unique_ptr<InstrumentationSession> m_CurrentSession;
    int m_ProfileCount;
    std::unordered_map<std::string, InstrumentTime> m_FunctionTimes;
    std::shared_mutex m_SharedMutex;
    std::vector<ProfileResult> m_ProfileCache;

  public:
    Instrumentor();
    ~Instrumentor();

    void BeginSession(const std::string &name, const std::string &filepath = "results.json",
                      const bool write_profiles = true);
    void EndSession();
    void WriteProfile(const ProfileResult &result);
    void AddFunctionTime(const std::string &name, long long time);
    static Instrumentor &Get();
    static std::unordered_map<std::string, InstrumentTime> &GetFunctionTimes()
    {
        return Get().m_FunctionTimes;
    }
    int64_t GetRuntime() const;

  private:
    void AddProfileToCache(const ProfileResult &result);
};
