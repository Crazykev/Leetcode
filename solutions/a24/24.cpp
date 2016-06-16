/*************************************************************************
	> File Name: 24.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月16日 星期四 14时31分31秒
    Solution of algorithm 22.Swap Nodes in Pairs. Easy. 2.80%
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
    ListNode* swapPairs(ListNode* head) {
        if(head == NULL || head->next == NULL){
            return head;
        }
        ListNode* cur = head, *next = head->next;
        ListNode* pre = head;
        cur->next = next->next;
        next->next = cur;
        head = next;
        pre = cur;
        cur = cur->next;
        while(cur != NULL && cur->next != NULL){
                next = cur->next;
                cur->next = next->next;
                next->next = cur;
                pre->next = next;
                pre = cur;
                cur = cur->next;
        }
        return head; 
    }
};
