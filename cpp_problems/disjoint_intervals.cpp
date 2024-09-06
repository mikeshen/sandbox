#include "template.h"
#include "test_runner.h"

// Given two list of disjoint intervals sorted on their start time, return a list of their intersections. 

// INFO
// the two lists are already sorted internally on start time
// 
// list1 = [[1, 3], [5, 6], [7, 9]]
// list2 = [[2, 3], [5, 7]]
// result = [2, 3], [5, 6], [7, 7]

// list1 = [[5, 10]]
// list2 = [[1, 3], [5, 7], [9, 12]]
// result = [5, 7], [9, 10]

// list1 = [[1, 10]]
// list2 = [[1, 3], [4, 7]]
// result = [[1, 3], [4, 7]]

using namespace std;


vector<vector<int>> intervalIntersetions(vector<vector<int>>& list1, vector<vector<int>>& list2) {
  vector<vector<int>> result;
  size_t i = 0, j = 0;

  while (i < list1.size() && j < list2.size()) {
    int start1 = list1[i][0], end1 = list1[i][1];
    int start2 = list2[j][0], end2 = list2[j][1];

    // Check if intervals intersect
    if (start1 <= end2 && start2 <= end1) {
      // calculate our intersection
      int start = max(start1, start2);
      int end = min(end1, end2);
      result.push_back({start, end});
    }

    // move the pointer that points to the interval that ends first
    if (end1 < end2) {
      i++;
    }
    else {
      j++;
    }
  }

  return result;
}

// list1 = [[1, 3], [5, 6], [7, 9]]
// list2 = [[2, 3], [5, 7]]
// result = [2, 3], [5, 6], [7, 7]

// list1 = [[5, 10]]
// list2 = [[1, 3], [5, 7], [9, 12]]
// result = [5, 7], [9, 10]

// list1 = [[1, 10]]
// list2 = [[1, 3], [4, 7]]
// result = [[1, 3], [4, 7]
// dummy test function example

void testFunction() {
  vector<vector<int>> list1 = {{1,3}, {5,6}, {7,9}};
  vector<vector<int>> list2 = {{2,3},{5,7}};
  vector<vector<int>> result = {{2, 3}, {5, 6}, {7, 7}};
  customAssert(intervalIntersetions(list1, list2) == result)
  auto vec = intervalIntersetions(list1, list2);
}

void testDisjoint() {
  vector<vector<int>> list1 = {{5,10}};
  vector<vector<int>> list2 = {{1,3},{5,7},{9,12}};
  vector<vector<int>> result = {{5, 7}, {9, 10}};
  customAssert(intervalIntersetions(list1, list2) == result)
  auto vec = intervalIntersetions(list1, list2);
}

void testGiantIntervalForList1() {
  vector<vector<int>> list1 = {{1,10}};
  vector<vector<int>> list2 = {{1,3},{4,7}};
  vector<vector<int>> result = {{1, 3}, {4, 7}};
  customAssert(intervalIntersetions(list1, list2) == result)
  auto vec = intervalIntersetions(list1, list2);
}

void testEmptyList2() {
  vector<vector<int>> list1 = {{1,10}};
  vector<vector<int>> list2 = {};
  vector<vector<int>> result = {};
  customAssert(intervalIntersetions(list1, list2) == result)
  auto vec = intervalIntersetions(list1, list2);
}

// Testing function runner
void runTests() {
    vector<string> testResults;

    // Add test functions here
    testResults.push_back(runTest("testFunction", testFunction));
    testResults.push_back(runTest("testDisjoint", testDisjoint));
    testResults.push_back(runTest("testGiantIntervalForList1", testGiantIntervalForList1));
    testResults.push_back(runTest("testEmptyList2", testEmptyList2));

    // Print test results
    for (const auto& result : testResults) {
        cout << result << endl;
    }
}

int main() {
    runTests();

  return 0;
}


// Your previous Plain Text content is preserved below:

