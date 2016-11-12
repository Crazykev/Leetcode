package main

import "fmt"

func twoSum(nums []int, target int) []int {
	numLen := len(nums)
	for i, num := range nums[:numLen-1] {
		tmpT := target - num
		for j, numb := range nums[i+1:] {
			if numb == tmpT {
				return []int{i, j + i + 1}
			}
		}
	}
	return nil
}
