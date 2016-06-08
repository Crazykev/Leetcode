/*************************************************************************
	> File Name: 14.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月08日 星期三 13时16分02秒
    Solution of algorithm 14. Longest Common Prefix Easy 47.92%
 ************************************************************************/

#include<iostream>
#include<vector>
using namespace std;

class Solution {
    public:
    string longestCommonPrefix(vector<string>& strs) {
        int str_num = strs.size();
        if(str_num == 0){
            return "";
        }else if (str_num == 1){
            return strs[0];
        }
        char curWord;
        bool first = true;
        for(int i=0; ; i++){
            if(first){
                if(strs[0].length() == 0){
                    return "";
                }
            }
            if(first && i!=0){
                first = false;
            }
            curWord = strs[0][i];
            for(int j=1; j<str_num; j++){
                if(first && strs[j].length() == 0){
                    return "";
                }
                if(strs[j][i] != curWord){
                    return strs[j].substr(0, i);
                }
            }
        }
        return strs[0];
    }
};
