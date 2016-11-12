## golang

Current Best: **69.58%** 

From solution runtime consume we can see, there are typically two types of solution, the first one is like my first one, Brute Force with time:O(n^2) and space O(1).  And the second one is hash map to deduce one dimision loop, with time O(n^2), space O(n).

History:
1. Just a normal solution with spent O(n^2). I found that using tmp num in first loop will deduce some time spent. And `for range` style is a little bit faster that just for each index style in golang, this somehow suprise me a bit.
2. Use a golang inside map, which is implement by hash table, so just use it as a hash table. First loop store nums as key and index as value in map, and the second loop try to get the complement num from map, if exist and value is not current index, then current num and map key is the wanted one.
Then I just notice that we don't need two loops, just can finish it in one loop, first check if wanted one is in map, if do, return, else insert current one into map and go to next loop. While this seems not deduce running time significantly.
