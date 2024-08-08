#include "template.h"

using namespace std;
/*
// DESCRIPTION
reconcile the transactions between d0 and d1

we're looking at D0 and transactions as like the source of
truth. We want to find the delta on what D1 shows vs that.

*/

unordered_map<string, double> parsePositions(const vector<string>& positions) {
    unordered_map<string, double> posMap;
    for (const auto& pos : positions) {
        istringstream ss(pos);
       string symbol;
        double amount;
        ss >> symbol >> amount;
        posMap[symbol] = amount;
    }
    return posMap;
}

vector<string> reconcilePositions(
    vector<string> d0_pos,
    vector<string> d1_trn,
    vector<string> d1_pos
) {
    vector<string> discrepancies;
    // 1. parse positions
    unordered_map<string, double> d0Map = parsePositions(d0_pos);
    unordered_map<string, double> d1Map = parsePositions(d1_pos);
    unordered_map<string, double> expectedMap = d0Map;

    // 2. process trxns
    for (const string& trn : d1_trn) {
        istringstream ss(trn);
        string symbol, type;
        int units;
        double value;
        ss >> symbol >> type >> units >> value;
        if (type == "BY") {
            expectedMap[symbol] += units;
            expectedMap["Cash"] -= value;
        }
        else if (type == "SL") {
            expectedMap[symbol] -= units;
            expectedMap["Cash"] += value;
        }
    }

    // 3. discrepancies
    for (const auto& kv : expectedMap) {
        string symbol = kv.first;
        double expectedAmount = kv.second;
        double actualAmount = d1Map[symbol];
        if (expectedAmount != actualAmount) {
            ostringstream discrepancy;
            discrepancy << symbol << " " << (actualAmount - expectedAmount);
            discrepancies.push_back((discrepancy.str()));
        }
    }
    // 4. check for positions in d1Map that were not in expected
    for (const auto& kv : d1Map) {
        string symbol = kv.first;
        if (expectedMap.find(symbol) == expectedMap.end()) {
            double actualAmount = kv.second;
            ostringstream discrepancy;
            discrepancy << symbol << " " << actualAmount;
            discrepancies.push_back((discrepancy.str()));
        }
    }

    return discrepancies;
}

string basic() {
    vector<string> d0_pos = {"AAPL 100", "GOOG 200", "Cash 10"};
    vector<string> d1_trn = {"AAPL SL 50 30000", "GOOG BY 10 10000"};
    vector<string> d1_pos = {"AAPL 50", "GOOG 220", "Cash 20000"};

    // reconcile positions
    vector<string> res = reconcilePositions(d0_pos, d1_trn, d1_pos);

    // display dis
    for (auto& dis : res)
        cout << dis << endl;
    return "basic";
}

/*
The custodial data provider has sent us a revision to the transactions data.
`DI-TRN-REVISED` completely overwrites `D1-TRN`. Adapt your code to handle the new input.

### Sample Input

    ["AAPL 100", "GOOG 200", "Cash 10"] # D0-POS

    ["AAPL SL 25 15000", "GOOG BY 20 10000", "AAPL SL 25 10000", "META SL 5 5000"]  ## DI-TRN-REVISED

    ["AAPL 50", "GOOG 220", "Cash 20000"] # D1-POS

### Sample Output

    ["Cash -10", "META 5"]  # revised output
*/
string revised() {
    vector<string> d0_pos = {"AAPL 100", "GOOG 200", "Cash 10"};
    vector<string> d1_trn = {"AAPL SL 25 15000", "GOOG BY 20 10000", "AAPL SL 25 10000", "META SL 5 5000"};
    vector<string> d1_pos = {"AAPL 50", "GOOG 220", "Cash 20000"};

    // reconcile positions
    vector<string> res = reconcilePositions(d0_pos, d1_trn, d1_pos);

    // display dis
    for (auto& dis : res)
        cout << dis << endl;
    return "revised";
}

string correct() {
    vector<string> d0_pos = {"AAPL 100", "GOOG 200", "Cash 10"};
    vector<string> d1_trn = {"AAPL SL 50 30000", "GOOG BY 10 10000"};
    vector<string> d1_pos = {"AAPL 50", "GOOG 210", "Cash 20010"};

    // reconcile positions
    vector<string> res = reconcilePositions(d0_pos, d1_trn, d1_pos);

    // display dis
    for (auto& dis : res)
        cout << dis << endl;
    return "correct";
}

string correctNoPositions() {
    vector<string> d0_pos = {};
    vector<string> d1_trn = {"AAPL BY 50 30000", "GOOG BY 10 10000"};
    vector<string> d1_pos = {"AAPL 50", "GOOG 10", "Cash -40000"};

    // reconcile positions
    vector<string> res = reconcilePositions(d0_pos, d1_trn, d1_pos);

    // display dis
    for (auto& dis : res)
        cout << dis << endl;
    return "correctNoPositions";
}

string discFirstTime() {
    vector<string> d0_pos = {};
    vector<string> d1_trn = {};
    vector<string> d1_pos = {"AAPL 50", "CASH 10000"};

    // reconcile positions
    vector<string> res = reconcilePositions(d0_pos, d1_trn, d1_pos);

    // display dis
    for (auto& dis : res)
        cout << dis << endl;
    return "discFirstTime";
}

void runTests() {
    string testname = basic();
    cout << testname << endl;
    testname = revised();
    cout << testname << endl;
    testname = correct();
    cout << testname << endl;
    testname = correctNoPositions();
    cout << testname << endl;
    testname = discFirstTime();
    cout << testname << endl;
}

int main() {
    runTests();
    return 0;
}
