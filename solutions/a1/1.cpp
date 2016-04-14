/*************************************************************************
	> File Name: 1.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月14日 星期四 18时18分07秒
    > Solve problem "1. Two Sum" Easy
 ************************************************************************/

#include<iostream>
#include<vector>

using namespace std;

class Solution {
    public:
    vector<int> twoSum(vector<int>& nums, int target) {
        vector<int> result;
        for(int i = 0; i < nums.size(); i++){
            for(int j = i+1; j < nums.size(); j++){
                if(nums[i] + nums[j] == target){
                    result.push_back(i);
                    result.push_back(j);
                    return result;
                }
            }
        }
        return result;
    }
};
