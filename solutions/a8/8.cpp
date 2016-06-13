/*************************************************************************
	> File Name: 8.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月13日 星期一 09时30分57秒
    Solution of algorithm 8. String to Integer(atoi) Easy 72.72%
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
    public:
    int myAtoi(string str) {
        string numStr;
        bool neg = false;
        int int_max = 2147483647;
        int int_min = -2147483648;
        if((numStr = validate(str)) == ""){
            return 0;
        }
        if(numStr == "--"){
            return int_min;
        }else if(numStr == "++"){
            return int_max;
        }
        if(numStr[0] == '-'){
            neg = true;
            numStr = numStr.substr(1, numStr.length()-1);
        }
        long long result = 0;
        int index = 0;
        int length = numStr.length();
        while(index < length){
            result *= 10;
            result += (numStr[index] - '0');
            if(index == length - 1){
                if(neg){
                    if(-result < int_min){
                        return int_min;
                    }
                }else{
                    if(result > int_max){
                        return int_max;
                    }
                }
            }
            index++;
        }
        if(neg){
            return -result;
        }
        return result;
    }

    string validate(string str){
        int index = 0;
        bool neg = false;
        int length = str.length();
        while(index < length && str[index] == ' '){index++;}
        if(index == length){
            return "";
        }
        if(str[index] == '-'){
            neg = true;
            index++;
        }else if(str[index] == '+'){
            index++;
        }
        int start = index;
        while(!(index == length || str[index] > '9' || str[index] < '0')){
            index++;
        }
        if(start == index){
            return "";
        }else{
            if(index - start > 10){
                return neg ? "--" : "++";
            }
            if(neg){
                return "-" + str.substr(start, index-start);
            }
            return str.substr(start, index-start);
        }
    }
};
