# Profiling

## How to Use Instrumentation

1. **Include the Instrumentation Header**  
   Include the header file `Profiler.hpp` in any file where you want to profile functions.

2. **Add Profiling Macros**  
   Use the `PROFILE_FUNCTION()` macro at the beginning of any function you want to profile. This macro will automatically record execution times for that function.

3. **Build with Profiling Enabled**  
   Ensure your project is built with the `PROFILING=1` flag. This enables profiling macros at compile time.
