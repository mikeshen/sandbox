#include "template.h"
#include "test_runner.h"
#include <cassert>

// Dynamic programming implementation of Fibonacci sequence
long long calcFib(int n) {
    const int size = 93; // Largest Fibonacci number storable in long long
    static vector<long long> dp(size + 1, -1); // Static storage for already calculated figures
    if (n == 0) {
        return 0;
    }
    else if (n == 1) {
        return 1;
    }
    else if (dp[n] != -1) {
        return dp[n];
    }
    else {
        dp[n] = calcFib(n - 1) + calcFib(n - 2);
    }
    return dp[n];
}

// dummy test function example
void testFunction() {
    assert(true); // Example test case
}

// Test function for calcFib
void testCalcFib() {
    // Test case 1
    assert(calcFib(0) == 0);
    // Test case 2
    assert(calcFib(1) == 1);
    // Test case 3
    assert(calcFib(2) == 1);
    // Test case 4
    assert(calcFib(6) == 8);
    // Test case 5
    assert(calcFib(10) == 55);
    // Test case 6
    assert(calcFib(20) == 6765);
    // Test case 7
    assert(calcFib(30) == 832040);
    // Test case 8
    assert(calcFib(40) == 102334155);
    // Test case 9
    assert(calcFib(50) == 12586269025);
    // Test case 10
    assert(calcFib(60) == 1548008755920);
}

// Testing function runner
void runTests() {
    vector<string> testResults;

    // Add test functions here
    testResults.push_back(runTest("testFunction", testFunction));
    testResults.push_back(runTest("testCalcFib", testCalcFib));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main() {
    runTests();
    return 0;
}