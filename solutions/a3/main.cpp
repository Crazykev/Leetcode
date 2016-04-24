/*************************************************************************
	> File Name: main.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月21日 星期四 21时36分18秒
 ************************************************************************/

#include<iostream>
#include"3.cpp"
using namespace std;

int main(){
    string str;
    Solution so;
    cin >> str;
    cout << str << endl;
    cout << so.lengthOfLongestSubstring(str) << endl;
    return 0;
}
