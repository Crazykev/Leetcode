/*************************************************************************
	> File Name: 7.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月25日 星期一 20时32分57秒
 ************************************************************************/

#include<iostream>
using namespace std;

class Solution {
    public:
    int reverse(int x) {
        bool neg;
        bool care;
        if(x==0){
            return 0;
        }else{
            neg = x<0;
        }
        int compare[]={2, 21, 214, 2147, 21474, 214748, 2147483, 21474836, 214748364, 2147483647};
        if(neg){
            x = 0 - x;
        }
        care = x > 1000000000;
        int result=0, tmp=0;
        bool first = true;
        for(int i=0; x>0; i++){
            tmp = x % 10;
            if(!first || tmp != 0){
                result = result*10;
                result += tmp;
                if( first && tmp != 0){
                   first = false; 
                }
            }
            if(care){
                if(result > compare[i]){
                    return 0;
                }else if(result < compare[i]){
                    care = false;
                }
            }
            x/=10;
        }
        if(neg){
            return -result;
        }else{
            return result;
        }
    }
};
