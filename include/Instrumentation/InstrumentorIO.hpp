#pragma once

#include "Instrumentation/InstrumentorUtils.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace InstrumentorIO
{
extern std::ofstream m_OutputStream;
extern std::shared_mutex m_SharedMutex;

void OpenFile(const std::string &filepath);
void CloseFile();
void WriteHeader(const std::string &sessionName);
void WriteFooter(const std::unordered_map<std::string, InstrumentTime> &functionTimes, int &profileCount);
void WriteCacheToFile(const std::vector<ProfileResult> &profileCache);
} // namespace InstrumentorIO
