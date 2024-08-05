#include <functional>
#include <string>
#include <exception>
#include <sstream>

#define customAssert(condition) \
    if (!(condition)) { \
        std::ostringstream oss; \
        oss << "Assertion failed: " << #condition; \
        throw std::logic_error(oss.str()); \
    }

// Helper function to run a test and catch exceptions
std::string runTest(const std::string& testName, std::function<void()> testFunc) {
    try {
        testFunc();
        return testName + ": PASSED";
    } catch (const std::logic_error& e) {
        return testName + ": FAILED with assertion: " + e.what();
    } catch (const std::exception& e) {
        return testName + ": FAILED with exception: " + e.what();
    } catch (...) {
        return testName + ": FAILED with unknown exception";
    }
}