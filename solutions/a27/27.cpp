/*************************************************************************
	> File Name: 27.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月23日 星期四 12时12分39秒
    Solution of algorithm 27. Remove Element. Easy 13.86%
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
public:
    int removeElement(vector<int>& nums, int val) {
        int begin = 0, size = nums.size(), end = size - 1;
        if(size == 0){
            return 0;
        }
        int repeat = 0;
        while(end >= begin){
            if(nums[begin] == val){
                while(end > begin && nums[end] == val){
                    end--;
                    repeat++;
                }
                if(end > begin){
                    nums[begin] = nums[end];
                    end--;
                    begin++;
                    repeat++;
                }else{
                    repeat++;
                    break;
                }
            }else{
                begin++;
            }
        }
        return size-repeat;
    }
};
