#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
using namespace std;

#define DEBUG 1
#ifdef DEBUG
#define DM(str) do { cout << str << endl; } while( false )
#else
#define DM(str) do { } while ( false )
#endif

//data types used often, but you don't want to type them time by time
#define ll long long
#define ull unsigned long long
#define ui unsigned int
#define us unsigned short

// vectors
typedef vector<int> vi;
typedef vector<vi> vvi;
typedef pair<int,int> pii;

//select the bit of position i of x
#define bit(x,i) (x&(1<<i))
//get the lowest bit of x
#define lowbit(x) ((x)&((x)^((x)-1)))
//get the highest bit of x, maybe the fastest
#define hBit(msb,n) asm("bsrl %1,%0" : "=r"(msb) : "r"(n))

//next four are for "for loops"
#define F(i,L,R) for (int i = L; i < R; i++)
#define FE(i,L,R) for (int i = L; i <= R; i++)
#define FF(i,L,R) for (int i = L; i > R; i--)
#define FFE(i,L,R) for (int i = L; i >= R; i--)

// container manipulation
#define fi first
#define se second
#define pb push_back
#define mp make_pair
// traverse an STL data structure
#define FOREACH(i,c) for (auto i=c.begin(); i!=c.end(); i++)
//handy for function like "sort()"
#define ALL(c) (c).begin(),(c).end()
#define PRESENT(c,x) ((c).find(x) != (c).end())
#define CPRESENT(c,x) (find(ALL(c),x) != (c).end())

// RANDOM
#define char2Int(c) (c-'0')
#define lastEle(vec) vec[vec.size()-1]
#define SZ(x) ((int)((x).size()))
// set a to the maximum of a and b
#define REMAX(a,b) (a)=max((a),(b))
#define REMIN(a,b) (a)=min((a),(b));

// directions
const int fx[4][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}};
const int fxx[8][2] = {{0,1}, {0,-1}, {1,0}, {-1,0}, {1,1}, {1,-1}, {-1,1}, {-1,-1}};

// Useful hardware instructions
#define bitcount _builtin_popcount
#define gcd _gcd
