#pragma once

#include "Instrumentation/InstrumentationTimer.hpp"
#include "Instrumentation/InstrumentorCore.hpp"
#include "Instrumentation/InstrumentorIO.hpp"
#include "Instrumentation/InstrumentorPrinting.hpp"

#if PROFILING
#ifdef _MSC_VER // Microsoft Compiler
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__FUNCTION__)
#else // Clang, GCC
#define PROFILE_FUNCTION() InstrumentationTimer timer##__LINE__(__PRETTY_FUNCTION__)
#endif
#else
#define PROFILE_FUNCTION()
#endif
