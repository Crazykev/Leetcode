/*************************************************************************
	> File Name: 3.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月21日 星期四 21时05分41秒
    > Solution of Algorithm problem 3. Longest Substring Without Repeating Characters. Medium
    > Cpp Radio: 49.80%
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
    public:
    int lengthOfLongestSubstring(string s) {
        int start=0,end=0;
        int exist[528]={0};
        int tmp=0, max=0, location=0, cur=0;
        if(s.length() == 0){
            return max;
        }
        exist[s[start]-'\0'] = 1;
        end = 1;
        cur = max = 1;
        while(end < s.length()){
            location = s[end] - '\0';
            if(exist[location] != 0){
                tmp = start + exist[location];
                for(int i=start; i<tmp; i++){
                    exist[s[i]-'\0'] = 0;
                }
                for(int i=tmp; i<=end; i++){
                    exist[s[i]-'\0'] = i - tmp + 1;
                }
                cur = end - tmp + 1;
                start = tmp;
            }else{
                exist[location] = end - start + 1;
                cur++;
            }
            if(cur > max){
                max = cur;
            }
            end++;
        }
        return max;
    }
};
