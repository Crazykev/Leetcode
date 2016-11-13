package main

import "fmt"

type HashTable struct {
	Table    []*Cell
	CellPool []Cell
	Current  int
	MagicNum int
}

type Cell struct {
	Value int
	Index int
	Next  *Cell
}

func NewCell(cp []Cell, index, key, value int) *Cell {
	c := &Cell{}
	//	c.Value[0] = value
	return c
}

func (c *Cell) Set(key, value int) {
	c.Value = key
	c.Index = value
}

func (c *Cell) Insert(nc *Cell) {
	var p *Cell = c
	for p.Next != nil {
		p = p.Next
	}
	p.Next = nc
}

func (c *Cell) Get(key int) (int, error) {
	var p *Cell = c
	for p.Value != key && p.Next != nil {
		p = p.Next
	}
	if p.Value != key {
		return -1, fmt.Errorf("Not exist")
	}
	return p.Index, nil
}

func NewHashTable(magicNum, size int) *HashTable {
	cp := make([]Cell, size)
	cs := make([]*Cell, magicNum)
	return &HashTable{
		Table:    cs,
		CellPool: cp,
		MagicNum: magicNum,
	}
}

func (h *HashTable) Insert(key, value int) {
	hashValue := h.hash(key)
	h.CellPool[h.Current].Set(key, value)
	if cell := h.Table[hashValue]; cell == nil {
		h.Table[hashValue] = &h.CellPool[h.Current]
	} else {
		cell.Insert(&h.CellPool[h.Current])
	}
	h.Current++
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
	table := NewHashTable(magicNum, numLen)
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
