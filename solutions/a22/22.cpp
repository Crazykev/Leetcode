/*************************************************************************
	> File Name: 22.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月15日 星期三 12时25分12秒
    Solution of algorithm 22. Generate Parentheses. Medium. 42.35%
 ************************************************************************/

#include<iostream>
#include<vector>

using namespace std;


class Solution {
    public:
    void dfs(vector<string> &list, char* cur, int minus, int remain, int len){
        if(remain <= 0){
            cur[len] = '\0';
            list.push_back(string(cur));
            return;
        }
        if(minus < remain){
            cur[len-remain]='(';
            dfs(list, cur, minus+1, remain-1, len);
        }
        if(minus > 0){
            cur[len-remain]=')';
            dfs(list, cur, minus-1, remain-1, len);
        }
    }
    vector<string> generateParenthesis(int n) {
        vector<string> result;
        char cur[1000];
        if(n < 0){
            return result;
        }
        cur[0] = '(';
        dfs(result, cur, 1, 2*n-1, 2*n);
        return result;
    }
};
