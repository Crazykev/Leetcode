## golang

Current Best: 72.18%

The intuition way to solve this problem is simulating add operation on paper, we iterate nums in both list and add them(and carry), the time complexity is O(max(m,n)). It's just fine, and we can't find a better way in theory.

### History

1. 1\_1 just implement it as upper description. It works fine, but run 32ms and rank 27.27%, which really sucks.

2. 2\_2 optimise complementation(%) operation. Consider the two num we add each time is non-negtive and less then 10, which means we can use compare and subtraction instead of %. This optimise help me get to 28ms and 78.18%.
