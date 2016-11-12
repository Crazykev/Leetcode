package main

import "fmt"

func twoSum(nums []int, target int) []int {
	numMap := make(map[int]int)
	for i, num := range nums {
		if index, ok := numMap[target-num]; ok {
			return []int{index, i}
		}
		numMap[num] = i
	}
	return nil
}
