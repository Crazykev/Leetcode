package main

import "fmt"

type ListNode struct {
	Val  int
	Next *ListNode
}

func newNode(val int) *ListNode {
	return &ListNode{
		Val: val,
	}
}

func add(val1, val2, carry int) (int, int) {
	res := val1 + val2 + carry
	if res >= 10 {
		return res - 10, 1
	}
	return res, 0
}

func addTwoNumbers(l1 *ListNode, l2 *ListNode) *ListNode {
	p1 := l1
	p2 := l2
	carry := 0
	sum := 0
	var resList, curList *ListNode
	if p1 == nil {
		return l2
	}
	if p2 == nil {
		return l1
	}
	for p1 != nil && p2 != nil {
		sum, carry = add(p1.Val, p2.Val, carry)
		if resList == nil {
			resList = newNode(sum)
			curList = resList
		} else {
			curList.Next = newNode(sum)
			curList = curList.Next
		}
		p1 = p1.Next
		p2 = p2.Next
	}
	for p1 != nil {
		sum, carry = add(p1.Val, 0, carry)
		curList.Next = newNode(sum)
		curList = curList.Next
		p1 = p1.Next
	}
	for p2 != nil {
		sum, carry = add(p2.Val, 0, carry)
		curList.Next = newNode(sum)
		curList = curList.Next
		p2 = p2.Next
	}
	if carry != 0 {
		curList.Next = newNode(carry)
	}
	return resList
}

func main() {
	fmt.Println("vim-go")
}
