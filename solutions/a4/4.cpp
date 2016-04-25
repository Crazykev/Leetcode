/*************************************************************************
	> File Name: 4.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月24日 星期日 14时57分06秒
    > Solution of Algorithm 6.ZigZag Conversion. Easy.  52.60%
 ************************************************************************/

#include<iostream>
#include<vector>
using namespace std;


class Solution {
    public:
    string convert(string s, int numRows) {
        if (numRows <= 1){
            return s;
        }
        string result = s;
        int size = s.length();
        vector<int> middle(size+1, 0);
        int gap = 2*(numRows - 1);
        int offset = 0;
        for(; offset <= (size-1)/gap; offset++){
            result[offset] = s[offset*gap];
            middle[offset] = offset*gap;
        }
        middle[offset] = (offset)*gap;
        for(int row=1; row < numRows-1; row++){
            for(int i=0; middle[i]<size-row; i++){
                result[offset++] = s[middle[i]+row];
                if(middle[i+1]<size+row){
                    result[offset++] = s[middle[i+1]-row];
                }
            }
        }
        for(int i=0; middle[i]<size-numRows; i++){
            result[offset++] = s[middle[i]+numRows-1];
        }
        return result;
    }
};
