# Sum of Permutations with Profiling

This project demonstrates two methods to compute the sum of permutations of a string of digits: a **non-optimized recursive approach** and an **optimized approach**. It also includes performance profiling using a custom **Instrumentation** system.

## How to Use Instrumentation

1. **Include the Instrumentation Header**  
   Include the header file `Instrumentation/Instrumentor.hpp` in any file where you want to profile functions.

2. **Add Profiling Macros**  
   Use the `PROFILE_FUNCTION()` macro at the beginning of any function you want to profile. This macro will automatically record execution times for that function.

3. **Set Up Instrumentation in `main()`**  
   Ensure your `main` function includes the following lines:
   ```cpp
   Instrumentor::Get().BeginSession("Profile", "profile.json", false);
   // Your code goes here.
   Instrumentor::Get().EndSession();
   ```

4. **Print Profiling Data (Optional)**  
   To output profiling data for review, include:
   ```cpp
   InstrumentorPrinting::PrintSessionData(Instrumentor::Get().GetFunctionTimes(), Instrumentor::Get().GetRuntime());
   ```

## Profiling Functions

- To profile any specific function in your project:
  - Include the `Instrumentation/Instrumentor.hpp` header in the corresponding file.
  - Add the macro `PROFILE_FUNCTION();` at the beginning of the function body.

  Example:
  ```cpp
  void ExampleFunction() {
      PROFILE_FUNCTION();
      // Function logic here
  }
  ```

## Profiling Setup in `main.cpp`

1. Include the `Instrumentation` setup lines in the `main()` function.
2. Use `BeginSession` to start the profiling session and `EndSession` to terminate it.

## Output

- Profiling data will be saved in a file named `profile.json`.
- You can review the profiling data for detailed insights into function execution times.
