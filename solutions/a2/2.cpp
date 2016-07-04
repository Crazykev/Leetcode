/*************************************************************************
	> File Name: 2.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年04月17日 星期日 21时25分37秒
    Solution of algorithm 2.Add Two Numbers. Easy. 69.47%
 ************************************************************************/

#include<iostream>
#include"2.h"

using namespace std;


 class Solution {
    public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode *l1Current = NULL, *l2Current = NULL;
        ListNode *result = NULL, *rCurrent = NULL;
        l1Current = l1;
        l2Current = l2;
        int overflow = 0, tmpSum = 0;
        while (l1Current != NULL && l2Current != NULL){
            tmpSum = l1Current->val + l2Current->val + overflow;
            if(result == NULL){
                result = new ListNode(tmpSum % 10);
                rCurrent = result;
            }else{
                rCurrent->next = new ListNode(tmpSum % 10);
                rCurrent = rCurrent->next;
            }
            overflow = tmpSum / 10;
            l1Current = l1Current->next;
            l2Current = l2Current->next;
        }
        ListNode *restCurrent = NULL;
        if(l1Current != NULL){
            restCurrent = l1Current;
        }else if (l2Current != NULL){
            restCurrent = l2Current;
        }
        while(restCurrent != NULL){
            tmpSum = restCurrent->val + overflow;
            if(result == NULL){
                result = new ListNode(tmpSum % 10);
                rCurrent = result;
            }else{
                rCurrent->next = new ListNode(tmpSum % 10);
                rCurrent = rCurrent->next;
            }
            overflow = tmpSum / 10;
            restCurrent = restCurrent->next;
        }
        if(overflow != 0){
            rCurrent->next = new ListNode(overflow);
        }
        return result;
    }
 };
