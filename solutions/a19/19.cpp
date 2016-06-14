/*************************************************************************
	> File Name: 19.cpp
	> Author: Crazykev
	> Mail: zcq8989@gmail.com
	> Created Time: 2016年06月14日 星期二 11时14分55秒
    Solution of algorithm 19. Remove Nth Node From End of List. Easy. 32.87%
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
    int countLen(ListNode* head, ListNode** buf, int level){
        int len = 0;
        int curLevel = 0;
        int curBuf = 0;
        if(level != 1){
            while(head != NULL){
                curLevel++;
                if(curLevel == level){
                    buf[curBuf++] = head;
                    curLevel = 0;
                }
                len++;
                head = head->next;
            }
        }else{
            while(head != NULL){
                buf[len++] = head;
                head = head->next;
            }
        }
        return len;
    }
    ListNode* removeNthFromEnd(ListNode* head, int n) {
        ListNode* buf[100];
        int listLen = 0;
        int depth = n/100 + 1;
        listLen = countLen(head, buf, depth);
        if(listLen == n){
            return head->next;
        }
        ListNode* searchStart = buf[(listLen-n-1)/depth];
        int miss = (listLen-n-1) % depth;
        for(int i =0; i<miss; i++){
            searchStart = searchStart->next;
        }
        searchStart->next = searchStart->next->next;
        return head;
    }
    
};
