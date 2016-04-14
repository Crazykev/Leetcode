/*************************************************************************
	> File Name: main.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月14日 星期四 19时58分13秒
 ************************************************************************/

#include<iostream>
#include<vector>
#include"1.cpp"

using namespace std;

int main(){
    vector<int> nums;
    int target;
    int count, num;
    
    cin >> count;
    for(int i = 0; i < count; i++){
        cin >> num;
        nums.push_back(num);
    }
    cin >> target;
    
    Solution s;
    vector<int> result = s.twoSum(nums, target);
    cout << "[" << result[0] << ", " << result[1] << "]" << endl;
    return 0;
}
