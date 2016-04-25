/*************************************************************************
	> File Name: main.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月24日 星期日 15时45分05秒
 ************************************************************************/

#include<iostream>
#include"4.cpp"
using namespace std;

int main(){
    string s;
    int numRows;
    cin >> s;
    cin >> numRows;
    Solution solution;
    cout << solution.convert(s, numRows) << endl;
    return 0;
}
