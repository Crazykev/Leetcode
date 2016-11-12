package main

import "fmt"

func twoSum(nums []int, target int) []int {
	numLen := len(nums)
	numMap := make(map[int]int, numLen)
	for i, num := range nums {
		numMap[num] = i
	}
	for i, num := range nums {
		if index, ok := numMap[target-num]; ok && index != i {
			return []int{i, index}
		}
	}
	return nil
}
