#include <vector>
#include <iostream>
#include <functional>
#include <string>
#include <exception>
#include <sstream>

using namespace std;

#define customAssert(condition) \
    if (!(condition)) { \
        ostringstream oss; \
        oss << "Assertion failed: " << #condition; \
        throw logic_error(oss.str()); \
    }

// Helper function to run a test and catch exceptions
string runTest(const string& testName, function<void()> testFunc) {
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
void testFunction() {
    customAssert(true); // Example test case
}

void addTwo() {
    customAssert(2+2==4); // Example test case
}

// Testing function runner
void runTests() {
    vector<string> testResults;

    // Add test functions here
    testResults.push_back(runTest("testFunction", testFunction));
    testResults.push_back(runTest("addTwo", addTwo));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main() {
    runTests();
    return 0;
}
