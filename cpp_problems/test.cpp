#include <exception>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

#define customAssert(condition)                    \
    if (!(condition)) {                            \
        ostringstream oss;                         \
        oss << "Assertion failed: " << #condition; \
        throw logic_error(oss.str());              \
    }

// Helper function to run a test and catch exceptions
string runTest(const string& testName, function<void()> testFunc)
{
    try {
        testFunc();
        return testName + ": PASSED";
    } catch (const logic_error& e) {
        return testName + ": FAILED with assertion: " + e.what();
    } catch (const exception& e) {
        return testName + ": FAILED with exception: " + e.what();
    } catch (...) {
        return testName + ": FAILED with unknown exception";
    }
}

// dummy test function example
void testFunction()
{
    customAssert(true); // Example test case
}

void testTwo()
{
    int two = 2;
    customAssert(2 == two); // Example test case
}

// Testing function runner
void runTests()
{
    vector<string> testResults;

    // Add test functions here
    testResults.push_back(runTest("testFunction", testFunction));
    testResults.push_back(runTest("testTwo", testTwo));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main()
{
    runTests();
    return 0;
}
