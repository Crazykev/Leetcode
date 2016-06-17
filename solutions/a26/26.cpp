/*************************************************************************
	> File Name: 26.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月17日 星期五 14时05分53秒
    Solution of algorithm 26. Remove Duplicates from Sorted Array. Easy. 45.09%
************************************************************************/

#include<iostream>
using namespace std;

class Solution {
public:
    int removeDuplicates(vector<int>& nums) {
        int curOffset, tailOffset = 0, curValue;
        int len = nums.size();
        if(len == 0){
            return 0;
        }
        curValue = nums[0];
        curOffset = 1;
        while(tailOffset < len){
            if(nums[tailOffset] == curValue){
                tailOffset++;
            }else{
                nums[curOffset++] = curValue = nums[tailOffset];
            }
        }
        return curOffset;
    }
};
