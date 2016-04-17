/*************************************************************************
	> File Name: main.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月15日 星期五 21时13分43秒
 ************************************************************************/

#include<iostream>
#include"2.h"
#include"2.cpp"
using namespace std;

int main(){
    Solution s;
    ListNode *l1 = NULL, *l2 = NULL;
    ListNode *c1 = NULL, *c2 = NULL;
    int num1, num2 = 0;
    int curNum = 0;
    cin >> num1;
    for(int i=0; i<num1; i++){
        cin >> curNum;
        if(l1 == NULL){
            l1 = new ListNode(curNum);
            c1 = l1;
        }else{
            c1->next = new ListNode(curNum);
            c1 = c1->next;
        }
    }
    cin >> num2;
    for(int i=0; i<num2; i++){
        cin >> curNum;
        if(l2 == NULL){
            l2 = new ListNode(curNum);
            c2 = l2;
        }else{
            c2->next = new ListNode(curNum);
            c2 = c2->next;
        }
    }
    ListNode *result = s.addTwoNumbers(l1, l2);
    cout << "[";
    while(result != NULL){
        cout << result->val << ", ";
        result = result->next;
    }
    cout << "]" << endl;
    return 0;
}
