# main.py

def fibonacci(n, memo={}):
    if n in memo:
        return memo[n]
    if n <= 0:
        return 0
    elif n == 1:
        return 1
    memo[n] = fibonacci(n - 1, memo) + fibonacci(n - 2, memo)
    return memo[n]

def permutations(n, k):
    dp = [[0] * (k + 1) for _ in range(n + 1)]
    for i in range(n + 1):
        for j in range(min(i, k) + 1):
            if j == 0:
                dp[i][j] = 1
            else:
                dp[i][j] = dp[i - 1][j] + j * dp[i - 1][j - 1]
    return dp[n][k]

def factorial(n):
    dp = [1] * (n + 1)
    for i in range(2, n + 1):
        dp[i] = dp[i - 1] * i
    return dp[n]

def main():
    # Test cases for the Fibonacci function
    test_case_1 = 10
    test_case_2 = 35 
    print(f"Fibonacci of {test_case_1} is {fibonacci(test_case_1)}")
    print(f"Fibonacci of {test_case_2} is {fibonacci(test_case_2)}")

    # Test cases for the Permutations function
    perm_test_case_1 = (5, 3)
    perm_test_case_2 = (10, 4)
    print(f"Permutations of {perm_test_case_1[0]} taken {perm_test_case_1[1]} at a time is {permutations(*perm_test_case_1)}")
    print(f"Permutations of {perm_test_case_2[0]} taken {perm_test_case_2[1]} at a time is {permutations(*perm_test_case_2)}")

    # Test cases for the Factorial function
    fact_test_case_1 = 5
    fact_test_case_2 = 10
    print(f"Factorial of {fact_test_case_1} is {factorial(fact_test_case_1)}")
    print(f"Factorial of {fact_test_case_2} is {factorial(fact_test_case_2)}")

if __name__ == "__main__":
    main()