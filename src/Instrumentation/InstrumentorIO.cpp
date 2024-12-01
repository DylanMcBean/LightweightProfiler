#include "Instrumentation/InstrumentorIO.hpp"

namespace InstrumentorIO
{
std::ofstream m_OutputStream;
std::shared_mutex m_SharedMutex;

void OpenFile(const std::string &filepath)
{
    m_OutputStream.open(filepath);
    if (!m_OutputStream.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filepath);
    }
}

void CloseFile()
{
    std::shared_lock<std::shared_mutex> lock(m_SharedMutex);
    if (m_OutputStream.is_open())
    {
        m_OutputStream.close();
    }
}

void WriteHeader(const std::string &sessionName)
{
    std::lock_guard<std::shared_mutex> lock(m_SharedMutex);
    m_OutputStream << "{\"session\": \"" << sessionName << "\",";
    m_OutputStream << "\"traceEvents\": [";
    m_OutputStream.flush();
}

void WriteFooter(const std::unordered_map<std::string, InstrumentTime> &functionTimes, int &profileCount)
{
    std::lock_guard<std::shared_mutex> lock(m_SharedMutex);

    if (!functionTimes.empty())
    {
        m_OutputStream << ",";
    }

    for (auto it = functionTimes.begin(); it != functionTimes.end(); ++it)
    {
        if (profileCount++ > 0)
            m_OutputStream << ",";
        m_OutputStream << "{";
        m_OutputStream << "\"cat\": \"Function Call Count\",";
        m_OutputStream << "\"args\":{\"Count\":" << it->second.Count << "},";
        m_OutputStream << "\"name\":\"_c " << it->first << "\",";
        m_OutputStream << "\"ph\": \"C\",";
        m_OutputStream << "\"pid\": 65536,";
        m_OutputStream << "\"tid\": 0,";
        m_OutputStream << "\"ts\":" << it->second.Start;
        m_OutputStream << "}";
    }

    for (auto it = functionTimes.begin(); it != functionTimes.end(); ++it)
    {
        m_OutputStream << ",";
        m_OutputStream << "{";
        m_OutputStream << "\"cat\": \"Function Total Runtime\",";
        m_OutputStream << "\"dur\":" << it->second.TotalTime << ',';
        m_OutputStream << "\"name\":\"_tr " << it->first << "\",";
        m_OutputStream << "\"ph\": \"X\",";
        m_OutputStream << "\"pid\": 0,";
        m_OutputStream << "\"tid\": 1,";
        m_OutputStream << "\"ts\":" << it->second.Start;
        m_OutputStream << "}";
    }

    m_OutputStream << "]}";
    m_OutputStream.flush();
}

void WriteCacheToFile(const std::vector<ProfileResult> &profileCache)
{
    if (profileCache.empty())
        return;

    bool isFirstProfile = profileCache.size() == 0;
    for (const auto &result : profileCache)
    {
        if (!isFirstProfile)
        {
            m_OutputStream << ",";
        }
        isFirstProfile = false;

        std::string name = result.Name;
        std::replace(name.begin(), name.end(), '"', '\'');

        m_OutputStream << "{";
        m_OutputStream << "\"cat\": \"Function Runtime\",";
        m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
        m_OutputStream << "\"name\":\"" << name << "\",";
        m_OutputStream << "\"ph\": \"X\",";
        m_OutputStream << "\"pid\": 0,";
        m_OutputStream << "\"tid\":" << result.ThreadID << ",";
        m_OutputStream << "\"ts\":" << result.Start;
        m_OutputStream << "}" << std::endl;
    }

    m_OutputStream.flush();
}
} // namespace InstrumentorIO
