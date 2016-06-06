/*************************************************************************
	> File Name: a13.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月06日 星期一 09时51分29秒
    Solution of algorithm 13.Roman to Integer Easy. 72.28%
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
public:
    int romanToInt(string s) {
        int value[] = {1, 5, 10, 50, 100, 500, 1000};
        int str_len = s.length();
        int current = 0;
        int curOrder = 0, nextOrder = 0;
        int result = 0;
        while(current < str_len){
            curOrder = getOrder(s[current]);
            if(current < str_len - 1 && (nextOrder = getOrder(s[current+1])) > curOrder){
                result += (value[nextOrder] - value[curOrder]);
                current += 2;
            }else{
                result += value[curOrder];
                current ++;
            }
        }
        return result;
    }
    
    int getOrder(char symbol){
        switch(symbol){
            case 'I':
            return 0;
            case 'V':
            return 1;
            case 'X':
            return 2;
            case 'L':
            return 3;
            case 'C':
            return 4;
            case 'D':
            return 5;
            case 'M':
            return 6;
            default:
            return -1;
        }
        return -1;
    }
};
