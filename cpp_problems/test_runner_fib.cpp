#include <vector>
#include <iostream>
#include "test_runner.h"

using namespace std;

// Dynamic programming implementation of Fibonacci sequence
unsigned long long calcFib(int n) {
    const int size = 93; // Largest Fibonacci number storable in long long
    static vector<long long> dp(size + 1, -1); // Static storage for already calculated figures
    if (dp[0] == -1) dp[0] = 0; // Initialize base case
    if (dp[1] == -1) dp[1] = 1; // Initialize base case
    if (dp[n] != -1)
        return dp[n];
    for (int i = 2; i <= n; ++i)
        if (dp[i] == -1)
            dp[i] = dp[i - 1] + dp[i - 2];

    return dp[n];
}

// dummy test function example
void testFunction() {
    customAssert(true); // Example test case
}

void testTwo() {
    int two = 2;
    customAssert(2 == two); // Example test case
}

// Test function for calcFib
void testCalcFib() {
    // Test case 1
    customAssert(calcFib(0) == 0);
    // Test case 2
    customAssert(calcFib(1) == 1);
    // Test case 3
    customAssert(calcFib(2) == 1);
    // Test case 4
    customAssert(calcFib(6) == 8);
    // Test case 5
    customAssert(calcFib(10) == 55);
    // Test case 6
    customAssert(calcFib(20) == 6765);
    // Test case 7
    customAssert(calcFib(30) == 832040);
    // Test case 8
    customAssert(calcFib(40) == 102334155);
    // Test case 9
    customAssert(calcFib(50) == 12586269025);
    // Test case 10
    customAssert(calcFib(60) == 1548008755920);
    // Test case 11: second largest fib number
    customAssert(calcFib(92) == 7540113804746346429);
    // Test case 11: largest fib number
    customAssert(calcFib(93) == 12200160415121876738ULL);
}

// Testing function runner
void runTests() {
    vector<string> testResults;

    // Add test functions here
    testResults.push_back(runTest("testFunction", testFunction));
    testResults.push_back(runTest("testTwo", testTwo));
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
