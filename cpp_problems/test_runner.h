#include <functional>
#include <string>
#include <exception>

// Helper function to run a test and catch exceptions
std::string runTest(const std::string& testName, std::function<void()> testFunc) {
    try {
        testFunc();
        return testName + ": PASSED";
    } catch (const std::exception& e) {
        return testName + ": FAILED with exception: " + e.what();
    } catch (...) {
        return testName + ": FAILED with unknown exception";
    }
}