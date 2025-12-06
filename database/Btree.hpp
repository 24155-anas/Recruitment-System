#ifndef BTREE_H
#define BTREE_H

#include "BTreeNode.hpp"
#include "BlockManager.hpp"
#include <iostream>
#include <queue>
#include <vector>

class BTree {
private:
    BlockManager* blockManager;
    BTreeNode* root;
    
    BTreeNode* loadNode(int blockIndex) {
        if (blockIndex == -1) return nullptr;
        
        BTreeNode* node = new BTreeNode();
        char buffer[BLOCK_SIZE];
        blockManager->readBlock(blockIndex, buffer);
        node->deserialize(buffer);
        node->blockIndex = blockIndex;
        
        return node;
    }
    
    void saveNode(BTreeNode* node) {
        if (node == nullptr || !node->isDirty) return;
        
        char buffer[BLOCK_SIZE];
        node->serialize(buffer);
        blockManager->writeBlock(node->blockIndex, buffer);
        node->isDirty = false;
    }
    
    void splitChild(BTreeNode* parent, int childIndex) {
        BTreeNode* leftChild = loadNode(parent->diskPointers[childIndex]);
        if (!leftChild) return;
        
        BTreeNode* rightChild = new BTreeNode(leftChild->isLeaf);
        rightChild->blockIndex = blockManager->allocateBlock();
        
        int midIndex = (ORDER - 1) / 2;
        
        rightChild->numKeys = leftChild->numKeys - midIndex - 1;
        for (int i = 0; i < rightChild->numKeys; i++) {
            rightChild->indexEntries[i] = leftChild->indexEntries[midIndex + 1 + i];
        }
        
        if (!leftChild->isLeaf) {
            for (int i = 0; i <= rightChild->numKeys; i++) {
                rightChild->diskPointers[i] = leftChild->diskPointers[midIndex + 1 + i];
            }
        }
        
        leftChild->numKeys = midIndex;
        
        for (int i = parent->numKeys; i > childIndex; i--) {
            parent->indexEntries[i] = parent->indexEntries[i - 1];
        }
        
        for (int i = parent->numKeys + 1; i > childIndex + 1; i--) {
            parent->diskPointers[i] = parent->diskPointers[i - 1];
        }
        
        parent->indexEntries[childIndex] = leftChild->indexEntries[midIndex];
        parent->diskPointers[childIndex + 1] = rightChild->blockIndex;
        parent->numKeys++;
        
        leftChild->isDirty = true;
        rightChild->isDirty = true;
        parent->isDirty = true;
        
        saveNode(leftChild);
        saveNode(rightChild);
        saveNode(parent);
    }
    
    void insertNonFull(BTreeNode* node, const BTreeNode::IndexEntry& entry) {
        if (node->isLeaf) {
            node->insertNonFull(entry);
            saveNode(node);
        }
        else {
            int i = node->numKeys - 1;
            while (i >= 0 && entry.key < node->indexEntries[i].key) {
                i--;
            }
            i++;
            
            int childBlockIndex = node->diskPointers[i];
            BTreeNode* child = loadNode(childBlockIndex);
            
            if (child->isFull()) {
                splitChild(node, i);
                if (entry.key > node->indexEntries[i].key) {
                    i++;
                }
                childBlockIndex = node->diskPointers[i];
                child = loadNode(childBlockIndex);
            }
            
            insertNonFull(child, entry);
        }
    }
    
public:
    BTree(const std::string& filename) {
        blockManager = new BlockManager(filename);
        
        int rootIdx = blockManager->getRootBlockIndex();
        if (rootIdx == -1) {
            root = new BTreeNode(true);
            root->blockIndex = blockManager->allocateBlock();
            blockManager->setRootBlockIndex(root->blockIndex);
            root->isDirty = true;
            saveNode(root);
        }
        else {
            root = loadNode(rootIdx);
        }
    }
    
    ~BTree() {
        delete blockManager;
    }
    
    void insert(const BTreeNode::IndexEntry& entry) {
        if (root->isFull()) {
            BTreeNode* newRoot = new BTreeNode(false);
            newRoot->blockIndex = blockManager->allocateBlock();
            newRoot->diskPointers[0] = root->blockIndex;
            
            splitChild(newRoot, 0);
            
            this->root = newRoot;
            blockManager->setRootBlockIndex(root->blockIndex);
        }
        
        insertNonFull(root, entry);
    }
    
    bool search(int key, BTreeNode::IndexEntry& result) {
        BTreeNode* node = root;
        
        while (node != nullptr) {
            int idx = node->findKey(key);
            
            if (idx < node->numKeys && node->indexEntries[idx].key == key) {
                result = node->indexEntries[idx];
                return true;
            }
            
            if (node->isLeaf) {
                return false;
            }
            
            int childBlockIndex = node->diskPointers[idx];
            node = loadNode(childBlockIndex);
        }
        
        return false;
    }
    
    void printTree() {
        if (root == nullptr) {
            std::cout << "Empty tree" << std::endl;
            return;
        }
        
        std::queue<BTreeNode*> q;
        q.push(root);
        
        while (!q.empty()) {
            int levelSize = q.size();
            
            for (int i = 0; i < levelSize; i++) {
                BTreeNode* node = q.front();
                q.pop();
                
                node->print();
                
                if (!node->isLeaf) {
                    for (int j = 0; j <= node->numKeys; j++) {
                        if (node->diskPointers[j] != -1) {
                            BTreeNode* child = loadNode(node->diskPointers[j]);
                            q.push(child);
                        }
                    }
                }
            }
            std::cout << "---------------------" << std::endl;
        }
    }
};
#endif