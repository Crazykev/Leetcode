package main

import "fmt"

func lengthOfLongestSubstring(s string) int {
	hashTable := [256]int{}
	var (
		start   int = 0
		longest int = 0
		curLen  int = 0
	)
	if len(s) == 0 {
		return 0
	}
	for curIndex, curChar := range s {
		if oldIndex := hashTable[curChar] - 1; oldIndex != -1 {
			if curLen > longest {
				longest = curLen
			}
			curLen = curIndex - oldIndex
			for i := start; i < oldIndex; i++ {
				hashTable[s[i]] = 0
			}
			start = oldIndex + 1
			hashTable[curChar] = curIndex + 1
			continue
		}
		hashTable[curChar] = curIndex + 1
		curLen++

	}
	if curLen > longest {
		longest = curLen
	}
	return longest
}

func main() {
	str := "bbbbaaaa"
	fmt.Printf("%d\n", lengthOfLongestSubstring(str))
}
