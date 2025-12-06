#ifndef BTREENODE_H
#define BTREENODE_H

#include <cstdint>
#include <cstring>
#include <iostream>

const int BLOCK_SIZE = 4096;
const int ORDER = 10;

class BTreeNode {
public:
    struct IndexEntry {
        int32_t key;            // Candidate ID
        int64_t dataOffset;     // Offset in data file
        int32_t recordSize;     // Size of record
        
        IndexEntry() : key(-1), dataOffset(-1), recordSize(0) {}
        IndexEntry(int k, int64_t off, int sz) : key(k), dataOffset(off), recordSize(sz) {}
    };
    
    IndexEntry indexEntries[ORDER - 1];
    int numKeys;
    bool isLeaf;
    
    int diskPointers[ORDER];
    BTreeNode* memPointers[ORDER];
    
    int blockIndex;
    bool isDirty;
    
    BTreeNode(bool leaf = true) {
        isLeaf = leaf;
        numKeys = 0;
        blockIndex = -1;
        isDirty = false;
        
        for (int i = 0; i < ORDER; i++) {
            diskPointers[i] = -1;
            memPointers[i] = nullptr;
        }
    }
    
    int findKey(int key) {
        int idx = 0;
        while (idx < numKeys && indexEntries[idx].key < key) {
            idx++;
        }
        return idx;
    }
    
    bool isFull() {
        return numKeys == ORDER - 1;
    }
    
    void insertNonFull(const IndexEntry& entry) {
        // Check for duplicates
        for (int j = 0; j < numKeys; j++) {
            if (indexEntries[j].key == entry.key) {
                std::cout << "Duplicate key " << entry.key << " detected" << std::endl;
                return;
            }
        }
        
        int i = numKeys - 1;
        while (i >= 0 && indexEntries[i].key > entry.key) {
            indexEntries[i + 1] = indexEntries[i];
            i--;
        }
        
        indexEntries[i + 1] = entry;
        numKeys++;
        isDirty = true;
    }
    
    void serialize(char* buffer) const {
        memset(buffer, 0, BLOCK_SIZE);
        size_t offset = 0;
        
        memcpy(buffer + offset, &isLeaf, sizeof(isLeaf));
        offset += sizeof(isLeaf);
        
        memcpy(buffer + offset, &numKeys, sizeof(numKeys));
        offset += sizeof(numKeys);
        
        memcpy(buffer + offset, &blockIndex, sizeof(blockIndex));
        offset += sizeof(blockIndex);
        
        for (int i = 0; i < ORDER - 1; i++) {
            memcpy(buffer + offset, &indexEntries[i], sizeof(IndexEntry));
            offset += sizeof(IndexEntry);
        }
        
        memcpy(buffer + offset, diskPointers, sizeof(diskPointers));
        offset += sizeof(diskPointers);
    }
    
    void deserialize(char* buffer) {
        size_t offset = 0;
        
        memcpy(&isLeaf, buffer + offset, sizeof(isLeaf));
        offset += sizeof(isLeaf);
        
        memcpy(&numKeys, buffer + offset, sizeof(numKeys));
        offset += sizeof(numKeys);
        
        memcpy(&blockIndex, buffer + offset, sizeof(blockIndex));
        offset += sizeof(blockIndex);
        
        for (int i = 0; i < ORDER - 1; i++) {
            memcpy(&indexEntries[i], buffer + offset, sizeof(IndexEntry));
            offset += sizeof(IndexEntry);
        }
        
        memcpy(diskPointers, buffer + offset, sizeof(diskPointers));
        offset += sizeof(diskPointers);
        
        for (int i = 0; i < ORDER; i++) {
            memPointers[i] = nullptr;
        }
        
        isDirty = false;
    }
    
    void print() const {
        std::cout << "[Block " << blockIndex << "]: ";
        for (int i = 0; i < numKeys; i++) {
            std::cout << indexEntries[i].key << "(" 
                      << indexEntries[i].dataOffset << ") ";
        }
        if (!isLeaf) std::cout << " [Internal]";
        std::cout << std::endl;
    }
};
#endif