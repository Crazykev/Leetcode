/*************************************************************************
	> File Name: 9.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月01日 星期三 11时03分58秒
    Solution of 9. Palindrome Number  Easy 62.71%
 ************************************************************************/

#include<iostream>
#include<cstdio>
#include<cstring>
using namespace std;

class Solution {
    public:
    bool isPalindrome(int x) {
        if (x < 0) {
            return false;
        }
        if (x < 10){
            return true;
        }
        char num_str[30] = {'\0'};
        int errcode = sprintf(num_str, "%d", x);
        if(errcode == -1){
            return false;
        }
        int start = 0;
        int end = strlen(num_str)-1;
        while(start < end){
            if (num_str[start++] != num_str[end--]){
                return false;
            }
        }
        return true;
    }

};
