/*************************************************************************
	> File Name: 21.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月22日 星期三 13时19分53秒
    Solution of algorithm 21. Merge Two Sorted Lists. Easy. 72.76%
    ************************************************************************/

#include<iostream>
using namespace std;

/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    ListNode* mergeTwoLists(ListNode* l1, ListNode* l2) {
        if(l1 == NULL) return l2;
        if(l2 == NULL) return l1;
        if(l1->val < l2->val){
            l1->next = mergeTwoLists(l1->next, l2);
            return l1;
        } else{
            l2->next = mergeTwoLists(l1, l2->next);
            return l2;
        }
    }
};
