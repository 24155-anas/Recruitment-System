#ifndef BTREE_NODE_H
#define BTREE_NODE_H

#include <cstdint>
#include <cstring>
#include <iostream>

const int ORDER = 20;  //wese order kaafi zyada hona chahoye, but b tree operations check  krne k liye abhi kam rakha

//new structure for index entry
// isme key is user id ya cv id hogi
// blockNum is data file ka block number jahan record stored hai
// offset is us block ke andar offset jahan record start hota hai

struct IndexEntry {
    int32_t key;           // User ID or CV ID
    int32_t blockNum;      // Block number in data file
    int32_t offset;        // Offset within that block
    
    IndexEntry() : key(-1), blockNum(-1), offset(-1) {}
    IndexEntry(int32_t k, int32_t b, int32_t o) : key(k), blockNum(b), offset(o) {}
    
    bool operator<(const IndexEntry& other) const {
        return key < other.key;
    }

    bool operator >(const IndexEntry& other) const {
        return key > other.key;
    }
    
    bool operator==(const IndexEntry& other) const {
        return key == other.key;
    }
};

class BTreeNode {
public:
    bool isLeaf;
    int32_t numKeys;
    IndexEntry entries[ORDER - 1];
    int32_t children[ORDER];  // Block numbers of child nodes
    int32_t blockIndex;       // This node's block number
    
    BTreeNode(bool leaf = true) : isLeaf(leaf), numKeys(0), blockIndex(-1) {
        for (int i = 0; i < ORDER; i++) {
            children[i] = -1;
        }
    }
    
    bool isFull() const {
        return numKeys == ORDER - 1;
    }
    
    int findKeyIndex(int32_t key) const {
        int idx = 0;
        while (idx < numKeys && entries[idx].key < key) {
            idx++;
        }
        return idx;
    }
    
    // Serialize node to buffer
    void serialize(char* buffer) const {
        int block_size = 4096;
        memset(buffer, 0, block_size);
        size_t offset = 0;
        
        memcpy(buffer + offset, &isLeaf, sizeof(isLeaf));
        offset += sizeof(isLeaf);
        
        memcpy(buffer + offset, &numKeys, sizeof(numKeys));
        offset += sizeof(numKeys);
        
        memcpy(buffer + offset, &blockIndex, sizeof(blockIndex));
        offset += sizeof(blockIndex);
        
        memcpy(buffer + offset, entries, sizeof(entries));
        offset += sizeof(entries);
        
        memcpy(buffer + offset, children, sizeof(children));
    }
    
    // Deserialize node from buffer
    void deserialize(const char* buffer) {
        size_t offset = 0;
        
        memcpy(&isLeaf, buffer + offset, sizeof(isLeaf));
        offset += sizeof(isLeaf);
        
        memcpy(&numKeys, buffer + offset, sizeof(numKeys));
        offset += sizeof(numKeys);
        
        memcpy(&blockIndex, buffer + offset, sizeof(blockIndex));
        offset += sizeof(blockIndex);
        
        memcpy(entries, buffer + offset, sizeof(entries));
        offset += sizeof(entries);
        
        memcpy(children, buffer + offset, sizeof(children));
    }
    
    void print() const {
        std::cout << "[Block " << blockIndex << ", " 
                  << (isLeaf ? "LEAF" : "INTERNAL") << "]: ";
        for (int i = 0; i < numKeys; i++) {
            std::cout << entries[i].key << " ";
        }
        std::cout << std::endl;
    }
};

#endif