#pragma once



#include "BTreeNode.hpp"
#include "BlockManager.hpp"
#include <iostream>
#include <queue>

class BTree {
private:
    BlockManager* blockMgr;
    BTreeNode* root;
    int minKeys;  // Minimum keys = (ORDER - 1) / 2
    
    //load node from disk
    BTreeNode* loadNode(int32_t blockNum) {
        if (blockNum == -1) {
            return nullptr;
        }
     
        BTreeNode* node = new BTreeNode();
        char buffer[BLOCK_SIZE];
        blockMgr->read(blockNum, buffer);
        node->deserialize(buffer);
        node->blockIndex = blockNum;
        
        return node;
    }
    
    // save node to disk
    void saveNode(BTreeNode* node) {
        if (!node || node->blockIndex == -1) {
            return;
        }
        
        char buffer[BLOCK_SIZE];
        node->serialize(buffer);
        //write that node in file
        blockMgr->write(node->blockIndex, buffer);
    }
    
    // Split child node
    void splitChild(BTreeNode* parent, int childIdx) {
        BTreeNode* left = loadNode(parent->children[childIdx]);
        if (!left) return;
        
        BTreeNode* right = new BTreeNode(left->isLeaf);
        right->blockIndex = blockMgr->allocateBlock();
        
        int mid = (ORDER - 1) / 2;
        
        // Copy middle key (will go to parent)
        IndexEntry midKey = left->entries[mid];
        
        // Move right half to new node (keys after mid)
        right->numKeys = left->numKeys - mid - 1;
        for (int i = 0; i < right->numKeys; i++) {
            right->entries[i] = left->entries[mid + 1 + i];
        }
        
        // Move child pointers for internal nodes
        if (!left->isLeaf) {
            for (int i = 0; i <= right->numKeys; i++) {
                right->children[i] = left->children[mid + 1 + i];
            }
        }
        
        // Update left node - keep only keys before mid
        left->numKeys = mid;
        
        // CRITICAL: Clear the entries that were moved to right
        // This ensures old data doesn't persist
        for (int i = mid; i < ORDER - 1; i++) {
            left->entries[i] = IndexEntry(-1, -1, -1);
        }
        
        // Clear child pointers that were moved
        if (!left->isLeaf) {
            for (int i = mid + 1; i < ORDER; i++) {
                left->children[i] = -1;
            }
        }
        
        // Insert middle key into parent
        for (int i = parent->numKeys; i > childIdx; i--) {
            parent->entries[i] = parent->entries[i - 1];
        }
        parent->entries[childIdx] = midKey;  // Use the saved middle key
        
        for (int i = parent->numKeys + 1; i > childIdx + 1; i--) {
            parent->children[i] = parent->children[i - 1];
        }
        parent->children[childIdx + 1] = right->blockIndex;
        parent->numKeys++;
        
        saveNode(left);
        saveNode(right);
        saveNode(parent);
        
        delete left;
        delete right;
    }
    
    // Insert into non-full node
    void insertNonFull(BTreeNode* node, const IndexEntry& entry) {
        if (node->isLeaf) {
            // Insert into leaf
            int i = node->numKeys - 1;
            while (i >= 0 && entry.key < node->entries[i].key) {
                node->entries[i + 1] = node->entries[i];
                i--;
            }
            node->entries[i + 1] = entry;
            node->numKeys++;
            saveNode(node);
        } else {
            // Find child to insert into
            int i = node->numKeys - 1;
            while (i >= 0 && entry.key < node->entries[i].key) {
                i--;
            }
            i++;
            
            BTreeNode* child = loadNode(node->children[i]);
            
            if (child->isFull()) {
                splitChild(node, i);
                if (entry.key > node->entries[i].key) {
                    i++;
                }
                delete child;
                child = loadNode(node->children[i]);
            }
            
            insertNonFull(child, entry);
            delete child;
        }
    }
    
    //deletion helpers

    // Get predecessor (rightmost key in left subtree)
    IndexEntry getPredecessor(int32_t childBlock) {
        BTreeNode* curr = loadNode(childBlock);
        
        while (!curr->isLeaf) {
            int32_t nextBlock = curr->children[curr->numKeys];
            BTreeNode* next = loadNode(nextBlock);
            delete curr;
            curr = next;
        }
        
        IndexEntry pred = curr->entries[curr->numKeys - 1];
        delete curr;
        return pred;
    }
    
    // Get successor (leftmost key in right subtree)
    IndexEntry getSuccessor(int32_t childBlock) {
        BTreeNode* curr = loadNode(childBlock);
        
        while (!curr->isLeaf) {
            int32_t nextBlock = curr->children[0];
            BTreeNode* next = loadNode(nextBlock);
            delete curr;
            curr = next;
        }
        
        IndexEntry succ = curr->entries[0];
        delete curr;
        return succ;
    }
    
    // Borrow from previous sibling
    void borrowFromPrev(BTreeNode* parent, int idx) {
        BTreeNode* child = loadNode(parent->children[idx]);
        BTreeNode* sibling = loadNode(parent->children[idx - 1]);
        
        // Move key from parent to child
        for (int i = child->numKeys; i > 0; i--) {
            child->entries[i] = child->entries[i - 1];
        }
        
        child->entries[0] = parent->entries[idx - 1];
        
        // Move key from sibling to parent
        parent->entries[idx - 1] = sibling->entries[sibling->numKeys - 1];
        
        // Move child pointer if not leaf
        if (!child->isLeaf) {
            for (int i = child->numKeys + 1; i > 0; i--) {
                child->children[i] = child->children[i - 1];
            }
            child->children[0] = sibling->children[sibling->numKeys];
        }
        
        child->numKeys++;
        sibling->numKeys--;
        
        saveNode(child);
        saveNode(sibling);
        saveNode(parent);
        
        delete child;
        delete sibling;
    }
    
    // Borrow from next sibling
    void borrowFromNext(BTreeNode* parent, int idx) {
        BTreeNode* child = loadNode(parent->children[idx]);
        BTreeNode* sibling = loadNode(parent->children[idx + 1]);
        
        // Move key from parent to child
        child->entries[child->numKeys] = parent->entries[idx];
        
        // Move key from sibling to parent
        parent->entries[idx] = sibling->entries[0];
        
        // Shift sibling's keys
        for (int i = 0; i < sibling->numKeys - 1; i++) {
            sibling->entries[i] = sibling->entries[i + 1];
        }
        
        // Move child pointer if not leaf
        if (!child->isLeaf) {
            child->children[child->numKeys + 1] = sibling->children[0];
            for (int i = 0; i < sibling->numKeys; i++) {
                sibling->children[i] = sibling->children[i + 1];
            }
        }
        
        child->numKeys++;
        sibling->numKeys--;
        
        saveNode(child);
        saveNode(sibling);
        saveNode(parent);
        
        delete child;
        delete sibling;
    }
    
    // Merge child with sibling
    void merge(BTreeNode* parent, int idx) {
        BTreeNode* left = loadNode(parent->children[idx]);
        BTreeNode* right = loadNode(parent->children[idx + 1]);
        
        // Pull key from parent and merge with right sibling
        left->entries[left->numKeys] = parent->entries[idx];
        left->numKeys++;
        
        // Copy keys from right to left
        for (int i = 0; i < right->numKeys; i++) {
            left->entries[left->numKeys + i] = right->entries[i];
        }
        
        // Copy child pointers if not leaf
        if (!left->isLeaf) {
            for (int i = 0; i <= right->numKeys; i++) {
                left->children[left->numKeys + i] = right->children[i];
            }
        }
        
        left->numKeys += right->numKeys;
        
        // Remove key from parent
        for (int i = idx; i < parent->numKeys - 1; i++) {
            parent->entries[i] = parent->entries[i + 1];
        }
        
        // Remove child pointer from parent
        for (int i = idx + 1; i < parent->numKeys; i++) {
            parent->children[i] = parent->children[i + 1];
        }
        parent->children[parent->numKeys] = -1;
        
        parent->numKeys--;
        
        saveNode(left);
        saveNode(parent);
        
        // Free right node
        blockMgr->freeBlock(right->blockIndex);
        
        delete left;
        delete right;
    }
    
    // Fill child node if it has fewer than minKeys
    void fill(BTreeNode* parent, int idx) {
        // If previous sibling has more than minKeys, borrow from it
        if (idx != 0) {
            BTreeNode* prevSibling = loadNode(parent->children[idx - 1]);
            if (prevSibling->numKeys > minKeys) {
                delete prevSibling;
                borrowFromPrev(parent, idx);
                return;
            }
            delete prevSibling;
        }
        
        // If next sibling has more than minKeys, borrow from it
        if (idx != parent->numKeys) {
            BTreeNode* nextSibling = loadNode(parent->children[idx + 1]);
            if (nextSibling->numKeys > minKeys) {
                delete nextSibling;
                borrowFromNext(parent, idx);
                return;
            }
            delete nextSibling;
        }
        
        // Merge with sibling
        if (idx != parent->numKeys) {
            merge(parent, idx);
        } else {
            merge(parent, idx - 1);
        }
    }
    
    // Delete from leaf node
    void removeFromLeaf(BTreeNode* node, int idx) {
        for (int i = idx; i < node->numKeys - 1; i++) {
            node->entries[i] = node->entries[i + 1];
        }
        node->numKeys--;
        saveNode(node);
    }
    
    // Delete from internal node
    void removeFromNonLeaf(BTreeNode* node, int idx) {
        int32_t key = node->entries[idx].key;
        
        // Case 1: Left child has >= minKeys+1 keys
        BTreeNode* leftChild = loadNode(node->children[idx]);
        if (leftChild->numKeys > minKeys) {
            IndexEntry pred = getPredecessor(node->children[idx]);
            node->entries[idx] = pred;
            saveNode(node);
            delete leftChild;
            
            // Recursively delete predecessor
            BTreeNode* child = loadNode(node->children[idx]);
            removeFromNode(child, pred.key);
            delete child;
            return;
        }
        delete leftChild;
        
        // Case 2: Right child has >= minKeys+1 keys
        BTreeNode* rightChild = loadNode(node->children[idx + 1]);
        if (rightChild->numKeys > minKeys) {
            IndexEntry succ = getSuccessor(node->children[idx + 1]);
            node->entries[idx] = succ;
            saveNode(node);
            delete rightChild;
            
            // Recursively delete successor
            BTreeNode* child = loadNode(node->children[idx + 1]);
            removeFromNode(child, succ.key);
            delete child;
            return;
        }
        delete rightChild;
        
        // Case 3: Both children have minKeys, merge them
        merge(node, idx);
        
        // Recursively delete from merged child
        BTreeNode* mergedChild = loadNode(node->children[idx]);
        removeFromNode(mergedChild, key);
        delete mergedChild;
    }
    
    // Main deletion function
    void removeFromNode(BTreeNode* node, int32_t key) {
        int idx = node->findKeyIndex(key);
        
        // Key found in this node
        if (idx < node->numKeys && node->entries[idx].key == key) {
            if (node->isLeaf) {
                removeFromLeaf(node, idx);
            } else {
                removeFromNonLeaf(node, idx);
            }
            return;
        }
        
        // Key not in this node
        if (node->isLeaf) {
            // Key doesn't exist
            return;
        }
        
        // Key might be in subtree rooted at children[idx]
        bool isInSubtree = (idx == node->numKeys);
        
        BTreeNode* child = loadNode(node->children[idx]);
        
        // If child has only minKeys, fill it first
        if (child->numKeys == minKeys) {
            delete child;
            fill(node, idx);
            
            // After filling, the key might have moved
            if (isInSubtree && idx > node->numKeys) {
                idx--;
            }
            
            child = loadNode(node->children[idx]);
        }
        
        // Recurse to child
        removeFromNode(child, key);
        delete child;
    }
    
public:
    BTree(BlockManager* blockManager) : blockMgr(blockManager), root(nullptr) {
        minKeys = (ORDER - 1) / 2;
        
        int32_t rootBlock = blockMgr->getRootBlock();
        
        if (rootBlock == -1) {
            // Create new root
            root = new BTreeNode(true);
            root->blockIndex = blockMgr->allocateBlock();
            blockMgr->setRootBlock(root->blockIndex);
            saveNode(root);
        } else {
            // Load existing root
            root = loadNode(rootBlock);
        }
    }
    
    ~BTree() {
        if (root) {
            saveNode(root);
            delete root;
        }
    }
    
    // Insert entry
    void insert(const IndexEntry& entry) {
        if (root->isFull()) {
            BTreeNode* newRoot = new BTreeNode(false);
            newRoot->blockIndex = blockMgr->allocateBlock();
            newRoot->children[0] = root->blockIndex;
            
            splitChild(newRoot, 0);
            
            delete root;
            root = newRoot;
            blockMgr->setRootBlock(root->blockIndex);
        }
        
        insertNonFull(root, entry);
    }

    
    
    // Search for key
    bool search(int32_t key, IndexEntry& result) {
        BTreeNode* curr = root;
        
        while (curr) {
            int idx = curr->findKeyIndex(key);
            
            if (idx < curr->numKeys && curr->entries[idx].key == key) {
                result = curr->entries[idx];
                return true;
            }
            
            if (curr->isLeaf) {
                return false;
            }
            
            int32_t childBlock = curr->children[idx];
            BTreeNode* next = loadNode(childBlock);

            if (curr != root) {
                delete curr;
            }
            curr = next;
        }
        
        return false;
    }
    
    // Delete key
    void remove(int32_t key) {
        if (!root) return;
        
        removeFromNode(root, key);
        
        // If root is empty after deletion and has a child
        if (root->numKeys == 0) {
            if (!root->isLeaf && root->children[0] != -1) {
                int32_t oldRootBlock = root->blockIndex;
                int32_t newRootBlock = root->children[0];
                
                blockMgr->freeBlock(oldRootBlock);
                delete root;
                
                root = loadNode(newRootBlock);
                blockMgr->setRootBlock(newRootBlock);
            }
            // If root is leaf and empty, keep it (empty tree)
        }
    }
    
    // Print tree (with proper memory cleanup)
    void print() {
        if (!root) {
            std::cout << "Empty tree" << std::endl;
            return;
        }
        
        std::queue<int32_t> q;
        q.push(root->blockIndex);
        
        while (!q.empty()) {
            int levelSize = q.size();
            
            for (int i = 0; i < levelSize; i++) {
                int32_t blockNum = q.front();
                q.pop();
                
                BTreeNode* node = loadNode(blockNum);
                node->print();
                
                if (!node->isLeaf) {
                    for (int j = 0; j <= node->numKeys; j++) {
                        if (node->children[j] != -1) {
                            q.push(node->children[j]);
                        }
                    }
                }
                
                delete node;  // Clean up memory
            }
            std::cout << "---" << std::endl;
        }
    }
};

