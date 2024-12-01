#include "Instrumentation/Instrumentor.hpp"
#include <string>
#include <vector>
#include <iostream>

void GeneratePermutationsString(std::string& digits, int l, int r, long long& totalSum) {
    PROFILE_FUNCTION();
    if (l == r) {
        totalSum += std::stoll(digits);
    } else {
        for (int i = l; i <= r; i++) {
            std::swap(digits[l], digits[i]);
            GeneratePermutationsString(digits, l + 1, r, totalSum);
            std::swap(digits[l], digits[i]);
        }
    }
}

long long SumOfPermutationsString_NonOptimized(const std::string& number) {
    PROFILE_FUNCTION();
    std::string digits = number;
    long long totalSum = 0;
    GeneratePermutationsString(digits, 0, digits.size() - 1, totalSum);
    return totalSum;
}

long long SumOfPermutationsString_Optimized(const std::string& number) {
    PROFILE_FUNCTION();

    int n = number.size();
    long long factorial = 1;
    long long positionalSumFactor = 0;

    for (int i = 1; i < n; ++i) {
        factorial *= i;
    }

    long long multiplier = 1;
    for (int i = 0; i < n; ++i) {
        positionalSumFactor += multiplier;
        multiplier *= 10;
    }

    long long totalSum = 0;
    for (char ch : number) {
        totalSum += (ch - '0') * factorial * positionalSumFactor;
    }

    return totalSum;
}

int main() {
    Instrumentor::Get().BeginSession("Profile", "profile.json", false);

    std::string number = "123456789";

    long long sumNonOptimized = SumOfPermutationsString_NonOptimized(number);
    long long sumOptimized = SumOfPermutationsString_Optimized(number);

    printf("Sum of permutations (non-optimized): %lld\n", sumNonOptimized);
    printf("Sum of permutations (optimized): %lld\n", sumOptimized);

    InstrumentorPrinting::PrintSessionData(Instrumentor::Get().GetFunctionTimes(), Instrumentor::Get().GetRuntime());

    Instrumentor::Get().EndSession();
    return 0;
}
