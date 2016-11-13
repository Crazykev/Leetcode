package main

import "fmt"

const (
	BUCKET_SIZE = 3
)

type HashTable struct {
	Table    []*Cell
	MagicNum int
}

type Cell struct {
	Value   [BUCKET_SIZE]Pair
	Next    *Cell
	current int
}

type Pair struct {
	Key   int
	Value int
}

func NewCell(value Pair) *Cell {
	c := &Cell{}
	c.Value[0] = value
	return c
}

func (c *Cell) Insert(value Pair) {
	if c.current < BUCKET_SIZE-1 {
		c.current++
		c.Value[c.current] = value
	} else if c.Next != nil {
		c.Next.Insert(value)
	} else {
		c.Next = NewCell(value)
	}
}

func (c *Cell) Exist(key int) bool {
	i := 0
	for ; i <= c.current; i++ {
		if c.Value[i].Key == key {
			return true
		}
	}
	if i < BUCKET_SIZE-1 || c.Next == nil {
		return false
	}
	return c.Next.Exist(key)
}

func (c *Cell) Get(key int) (int, error) {
	i := 0
	for ; i <= c.current; i++ {
		if c.Value[i].Key == key {
			return c.Value[i].Value, nil
		}
	}
	if i < BUCKET_SIZE-1 || c.Next == nil {
		return -1, fmt.Errorf("Not Exist")
	}
	return c.Next.Get(key)
}

func NewHashTable(magicNum, size int) *HashTable {
	cs := make([]*Cell, size)
	return &HashTable{
		Table:    cs,
		MagicNum: magicNum,
	}
}

func (h *HashTable) Insert(key, value int) {
	hashValue := h.hash(key)
	if cell := h.Table[hashValue]; cell == nil {
		h.Table[hashValue] = NewCell(Pair{key, value})
	} else {
		cell.Insert(Pair{key, value})
	}
}

func (h *HashTable) Get(key int) (int, error) {
	hashValue := h.hash(key)
	if cell := h.Table[hashValue]; cell == nil {
		return 0, fmt.Errorf("Not Exist")
	} else {
		return cell.Get(key)
	}
}

func (h *HashTable) hash(value int) int {
	mod := value % h.MagicNum
	if mod < 0 {
		return -mod
	}
	return mod
}

func twoSum(nums []int, target int) []int {
	numLen := len(nums)
	magicNum := 701
	if numLen < magicNum {
		magicNum = numLen
	}
	table := NewHashTable(magicNum, magicNum)
	for i, num := range nums {
		if index, err := table.Get(target - num); err == nil {
			return []int{index, i}
		}
		table.Insert(num, i)
	}
	return nil
}

func main() {
	nums := []int{150, 24, 79, 50, 88, 345, 3}
	res := twoSum(nums, 200)
	fmt.Printf("%v\n", res)
}
