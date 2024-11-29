#include "template.h"
#include "test_runner.h"

string processString(const string& str)
{
    string result = str;

    // Remove non-whitespace and punctuation characters
    result.erase(remove_if(result.begin(), result.end(), [](char c) { return !isalnum(c); }),
                 result.end());

    // Convert to lowercase
    transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return tolower(c);
    });

    return result;
}

bool isPalindrome(string s)
{
    s = processString(s);
    if (s.size() == 1)
        return true;
    int i = 0, j = s.size() - 1;
    while (i <= j && s[i] == s[j]) {
        i++;
        j--;
    }
    return i > j;
}

void testPalindrome()
{
    customAssert(isPalindrome("racecar"));
    customAssert(isPalindrome("race car"));
}

void runTests()
{
    vector<string> testResults;
    testResults.push_back(runTest("testPalindrome", testPalindrome));

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
