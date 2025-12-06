#ifndef BLOCKMANAGER_H
#define BLOCKMANAGER_H

#include <fstream>
#include <cstring>
#include <iostream>
#include "BTreeNode.hpp"

const int MAX_BLOCKS = 1000;
const int BITMAP_SIZE = MAX_BLOCKS / 8;

class BlockManager {
private:
    std::fstream file;
    uint8_t bitmap[BITMAP_SIZE];
    int rootBlockIndex;
    std::string filename;
    
    bool isBitSet(int blockIndex) {
        int byteIndex = blockIndex / 8;
        int bitIndex = blockIndex % 8;
        return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
    }
    
    void setBit(int blockIndex) {
        int byteIndex = blockIndex / 8;
        int bitIndex = blockIndex % 8;
        bitmap[byteIndex] |= (1 << bitIndex);
    }
    
    void clearBit(int blockIndex) {
        int byteIndex = blockIndex / 8;
        int bitIndex = blockIndex % 8;
        bitmap[byteIndex] &= ~(1 << bitIndex);
    }
    
public:
    BlockManager(const std::string& fname) : filename(fname), rootBlockIndex(-1) {
        memset(bitmap, 0, BITMAP_SIZE);
        
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        
        if (!file.is_open()) {
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            
            setBit(0); // Block 0 for superblock
            writeSuperBlock();
        }
        else {
            readSuperBlock();
        }
    }
    
    ~BlockManager() {
        if (file.is_open()) {
            writeSuperBlock();
            file.close();
        }
    }
    
    int allocateBlock() {
        for (int i = 1; i < MAX_BLOCKS; i++) {
            if (!isBitSet(i)) {
                setBit(i);
                return i;
            }
        }
        return -1;
    }
    
    void deallocateBlock(int blockIndex) {
        if (blockIndex > 0 && blockIndex < MAX_BLOCKS) {
            clearBit(blockIndex);
        }
    }
    
    void readBlock(int blockIndex, char* buffer) {
        file.seekg(blockIndex * BLOCK_SIZE, std::ios::beg);
        file.read(buffer, BLOCK_SIZE);
    }
    
    void writeBlock(int blockIndex, const char* buffer) {
        file.seekp(blockIndex * BLOCK_SIZE, std::ios::beg);
        file.write(buffer, BLOCK_SIZE);
        file.flush();
    }
    
    void writeSuperBlock() {
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        
        int offset = 0;
        memcpy(buffer + offset, &rootBlockIndex, sizeof(int));
        offset += sizeof(int);
        
        memcpy(buffer + offset, bitmap, BITMAP_SIZE);
        
        writeBlock(0, buffer);
    }
    
    void readSuperBlock() {
        char buffer[BLOCK_SIZE];
        readBlock(0, buffer);
        
        int offset = 0;
        memcpy(&rootBlockIndex, buffer + offset, sizeof(int));
        offset += sizeof(int);
        
        memcpy(bitmap, buffer + offset, BITMAP_SIZE);
    }
    
    int getRootBlockIndex() { 
        return rootBlockIndex;
    }
    
    void setRootBlockIndex(int idx) {
        rootBlockIndex = idx;
        writeSuperBlock();
    }
};
#endif