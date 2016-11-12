package main

import "fmt"

func twoSum(nums []int, target int) []int {
	numLen := len(nums)
	for i := 0; i < numLen-1; i++ {
		tmpT := target - nums[i]
		for j := i + 1; j < numLen; j++ {
			if nums[j] == tmpT {
				return []int{i, j}
			}
		}
	}
	return nil
}
