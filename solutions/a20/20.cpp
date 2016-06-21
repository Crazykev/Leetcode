/*************************************************************************
	> File Name: 20.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月21日 星期二 17时17分36秒
    Solution of 20. Valid Parentheses. Easy 
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
public:
    bool isValid(string s) {
        int len = s.length();
        if(len % 2 == 1){
            return false;
        }else if(len == 0){
            return true;
        }
        int io[3] = {0};
        int kind;
        int start = 0, end = 1;
        stack<char> tower;
        for(int i=0; i<len; i++){
            if(isLeft(s[i])){
                tower.push(s[i]);
            }else{
                if(tower.empty() || !validate(tower.top(), s[i])){
                    return false;
                }else{
                    tower.pop();
                }
            }
        }
        if(tower.empty()){
            return true;
        }else{
            return false;
        }
    }
    
    bool validate(char a, char b){
        switch(b){
            case ')':
            if(a == '('){
                return true;
            }else{
                return false;
            }
            case ']':
            if(a == '['){
                return true;
            }else{
                return false;
            }
            case '}':
            if(a == '{'){
                return true;
            }else{
                return false;
            }
            default:
            return false;
        }
    }
    bool isLeft(char c){
        switch(c) {
            case '(':
            return true;
            case '[':
            return true;
            case '{':
            return true;
            default:
            return false;
        }
    }
};
