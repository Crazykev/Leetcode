package main

import "fmt"

func longestPalindrome(s string) string {
	strLen := len(s)
	curLen := 0
	center := strLen
	odd := (strLen & 1) != 0
	resultString := ""
	maxLen := getLongestPalindromeLength(s, odd)
	if maxLen != 0 {
		resultString = s[(center-maxLen)>>1 : (center+maxLen)>>1]
		if maxLen == center {
			return s
		}
	}
	for delta := 1; delta < strLen; delta++ {
		odd = !odd
		curLen = getLongestPalindromeLength(s[:center-delta], odd)
		if curLen > maxLen {
			maxLen = curLen
			resultString = s[(center-delta-curLen)>>1 : (center-delta+curLen)>>1]
		}
		curLen = getLongestPalindromeLength(s[delta:center], odd)
		if curLen > maxLen {
			maxLen = curLen
			resultString = s[(center+delta-curLen)>>1 : (center+delta+curLen)>>1]
			if maxLen > center-delta {
				return resultString
			}
		}
	}
	return resultString
}

func getLongestPalindromeLength(s string, odd bool) int {
	delta := 1
	if odd {
		delta = 0
	}
	center := len(s) / 2
	curLen := 0
	if odd {
		curLen = -1
	}
	for i := 0; center-i-delta >= 0 && s[center-i-delta] == s[center+i]; i++ {
		curLen += 2
	}
	return curLen
}

func main() {
	res := longestPalindrome("bbb")
	fmt.Printf("%s\n", res)
}
