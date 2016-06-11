/*************************************************************************
	> File Name: 12.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月08日 星期三 14时57分36秒
    Solution of algorithm 12. Integer to Roman Medium. 87.60% 
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
public:
    string intToRoman(int num) {
        int numMatrix[][4] = {
            {1000, 900, 500, 400},
            {100, 90, 50, 40},
            {10, 9, 5, 4},
            {1, 5000, 5000, 5000}
        };
        char symbolMatrix[][2] = {
            {'M', 'D'},
            {'C', 'L'},
            {'X', 'V'},
            {'I', '\0'},
        };
        char result[20] = {'\0'};
        int ptr = 0;
        for(int base=getStart(num); base < 4; base++){
            while(num >= numMatrix[base][0]){
                result[ptr++] = symbolMatrix[base][0];
                num -= numMatrix[base][0];
            }
            if(num >= numMatrix[base][1]){
                result[ptr++] = symbolMatrix[base+1][0];
                result[ptr++] = symbolMatrix[base][0];
                num -= numMatrix[base][1];
            }else if(num >= numMatrix[base][2]){
                result[ptr++] = symbolMatrix[base][1];
                num -= numMatrix[base][2];
            }else if(num >= numMatrix[base][3]){
                result[ptr++] = symbolMatrix[base+1][0];
                result[ptr++] = symbolMatrix[base][1];
                num -= numMatrix[base][3];
            }
        }
        return string(result);
    }
    int getStart(int num){
        if(num >= 400){
            return 0;
        }else if(num >= 40){
            return 1;
        }else if(num >= 4){
            return 2;
        }else{
            return 3;
        }
    }
};
