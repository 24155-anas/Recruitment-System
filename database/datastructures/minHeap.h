#pragma once
#include<iostream>
using namespace std;
#include <stdexcept>
template <class T>
class minHeap {
private:

    T* arr;
    int _size;
    int capacity;

    int getLeftChild(int parent) {
        //2i + 1

        return (2 * parent + 1);
    }
    int getRightChild(int parent) {
        //2i + 2

        return (2 * parent + 2);
    }
    int getParent(int child) {
        //i-1 / 2
        return ((child - 1) / 2);
    }

    //jaha pe last insert hoa ha
    //waha se check kro heap ka structure upwards
    //do recursively
    void heapifyUp(int node) {

        //node is the index jaha pe insert kia
        int idx = node;
        while (idx > 0) {
            if (arr[getParent(idx)] > arr[idx]) {
                swap(arr[idx], arr[getParent(idx)]);
            }
            else {
                break;
            }
            idx = getParent(idx);
        }
    }

    //check heap structure while goingdown
    void heapifyDown(int node) {


        //swap the parent with the smallest child
        int idx = node;
        while (idx < this->_size) {
            int rightChild = getRightChild(idx);
            int leftChild = getLeftChild(idx);

            //pehle smallest dhoondo
            int smallest = idx;
            if (leftChild < _size and arr[leftChild] < arr[smallest]) {
                smallest = leftChild;
            }
            if (rightChild < _size and arr[rightChild] < arr[smallest]) {
                smallest = rightChild;
            }
        
            if (idx == smallest) {
                break;
            }
            swap(arr[idx], arr[smallest]);

            idx = smallest;
        }

    }

public:

    //contructor
    minHeap() {
        this->capacity = 100;
        this->_size = 0;

        this->arr = new T[capacity];
    
    }

    //convert to heap
    void heapify(T* _arr, int _capacity) {
    
        for (int i = 0; i < _capacity; i++) {
            arr[i] = _arr[i];
        }
        this->_size = _capacity;

        for (int i = (_size / 2) - 1; i >= 0; i--) {
            heapifyDown(i);
        }

       
    }
    minHeap(T* _arr, int _capacity) {
        this->arr = new T[100];
        this->capacity = 100;
        this->_size = 0;
        heapify(_arr, _capacity);
    }


    T getMin() {
        return arr[0];
    }
    T extractMin() {
        //remove and return
        if (_size == 0) {
            cout << "Empty" << endl;
            throw runtime_error("EMPTY");
        }
        T min = arr[0];

        arr[0] = arr[_size - 1];
        this->_size--;

        if (_size > 0) {
            heapifyDown(0);
        }

        return min;
    }


    void insert(T _key) {
        //add new element
        if (_size < capacity) {
            arr[_size] = _key;
            _size++;
            //heapify up
            heapifyUp(_size - 1);
        }
        else {
            throw runtime_error("Heap is full, cannot insert more");
        }

    }
    void erase(T _key) {
        //find this key in heap
        //delete it
        //swap from the last heap elemnt
        //call heapify down from here
        int idx = -1;
        for (int i = 0; i < _size; i++) {
            if (arr[i] == _key) {
                idx = i;
                break;
            }
        }

        //swap with the last element
        arr[idx] = arr[_size - 1];
        this->_size--;

        //agr wo apne parent se chota ha to call heapify up 
        if (idx > 0 and arr[idx] < arr[getParent(idx)]) {
            heapifyUp(idx);
        }
        else {
            heapifyDown(idx);
        }
    }
    bool empty() {
        return _size == 0;
    }
    int size() {
        return this->_size;
    }
    static void sort(T* _arr, int _capacity) {
        //descending order sort
        minHeap<T> tempHeap(_arr, _capacity);

        for (int i = 0; i < _capacity; i++) {
            _arr[_capacity - i - 1] = tempHeap.extractMin();
        }

    }
    ~minHeap() {
        delete[] arr;
    }
};



