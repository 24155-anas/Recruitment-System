#ifndef BLOCK_MANAGER_H
#define BLOCK_MANAGER_H

#include <fstream>
#include <cstring>
#include <iostream>
#include <cstdint>
#include <iostream>

using namespace std;
const int BLOCK_SIZE = 4096;  // 4KB blocks for EVERYTHING
const int MAX_BLOCKS = 1000;
const int BITMAP_SIZE = MAX_BLOCKS / 8;  // 125 bytes

// hr file k super block me hoga
// File Header - stored in Block 0 of every file
struct FileHeader {
    char signature[8];           // "CVDBFILE"
    int32_t allocatedBlocksCount; // Number of blocks allocated
    int32_t rootBlockIndex;      // Root block for B-tree (index files only)
    int32_t recordCount;         // Total records (data files only)
    uint8_t bitmap[BITMAP_SIZE]; // Block allocation bitmap
    
    FileHeader() : allocatedBlocksCount(1), rootBlockIndex(-1), recordCount(0) {
        memset(signature, 0, sizeof(signature));
        strncpy(signature, "CVDBFILE", 7);
        memset(bitmap, 0, BITMAP_SIZE);
        setBit(0);  //block 0 is header block
    }
    
    bool isBitSet(int blockNum) const {
        if (blockNum < 0 || blockNum >= MAX_BLOCKS) return false;
        int byteIdx = blockNum / 8;
        int bitIdx = blockNum % 8;
        return (bitmap[byteIdx] & (1 << bitIdx)) != 0;
    }
    
    void setBit(int blockNum) {
        if (blockNum < 0 || blockNum >= MAX_BLOCKS) return;
        int byteIdx = blockNum / 8;
        int bitIdx = blockNum % 8;
        bitmap[byteIdx] |= (1 << bitIdx);
    }
    
    void clearBit(int blockNum) {
        if (blockNum < 0 || blockNum >= MAX_BLOCKS) return;
        int byteIdx = blockNum / 8;
        int bitIdx = blockNum % 8;
        bitmap[byteIdx] &= ~(1 << bitIdx);
    }
    
    int findFreeBlock() const {
        //0 is super block
        for (int i = 1; i < MAX_BLOCKS; i++) {
            if (!isBitSet(i)) return i;
        }
        return -1;
    }


};
std::ostream& operator<<(std::ostream& os, const FileHeader& header) {
    os << "Signature: " << header.signature << "\n";
    os << "Allocated Blocks: " << header.allocatedBlocksCount << "\n";
    os << "Root Block Index: " << header.rootBlockIndex << "\n";
    os << "Record Count: " << header.recordCount << "\n";

    os << "Bitmap (bits): ";
    for (int i = 0; i < BITMAP_SIZE; i++) {
        for (int bit = 0; bit < 8; bit++) {
            int bitValue = (header.bitmap[i] >> bit) & 1;
            os << bitValue;
        }
        os << ' ';
    }
    os << "\n";
    return os;
}




class BlockManager {
private:
    std::fstream file;
    std::string filename;
    FileHeader header;
    
    //read an entire block
    void readBlock(int32_t blockNum, char* buffer) {
        file.seekg(blockNum * BLOCK_SIZE, std::ios::beg);
        file.read(buffer, BLOCK_SIZE);
    }
    
    //write an entire block
    void writeBlock(int32_t blockNum, const char* buffer) {
        file.seekp(blockNum * BLOCK_SIZE, std::ios::beg);
        file.write(buffer, BLOCK_SIZE);
        file.flush();
    }
    
    //load header from file in our class header object
    void loadHeader() {
        char buffer[BLOCK_SIZE];
        readBlock(0, buffer);
        memcpy(&header, buffer, sizeof(FileHeader));
    }
    
    //header ko file me rakhwana
    void saveHeader() {
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        memcpy(buffer, &header, sizeof(FileHeader));
        writeBlock(0, buffer);
    }
    
public:

    void readFileHeader() {
        loadHeader();
        cout<<this->header;
    }

    BlockManager(const std::string& fname) : filename(fname) {
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        
        if (!file.is_open()) {
            // Create new file
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            
            header = FileHeader();
            saveHeader();
            std::cout << "Created: " << filename << std::endl;
        } else {
            loadHeader();
            
            if (strncmp(header.signature, "CVDBFILE", 7) != 0) {
                std::cerr << "Invalid file: " << filename << std::endl;
            } else {
                std::cout << "Loaded: " << filename << std::endl;
            }
        }
    }
    
    ~BlockManager() {
        if (file.is_open()) {
            saveHeader();
            file.close();
        }
    }
    

    //allocating a new block
    int32_t allocateBlock() {
        //bitmap me se free block dhoondo
        int32_t blockNum = header.findFreeBlock();
        if (blockNum == -1) {
            std::cerr << "No free blocks" << std::endl;
            return -1;
        }
        
        //us block ko allocate krdo
        header.setBit(blockNum);
        //and increase total blocks count
        header.allocatedBlocksCount++;
        //header ko file me bhi update krdo
        saveHeader();
        
        //block ko zero se initialize krdo
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        writeBlock(blockNum, buffer);
        
        return blockNum;
    }
    
    // Free a block
    void freeBlock(int32_t blockNum) {
        if (blockNum <= 0) return;
        
        //bitmap me us bit ko clear krdo
        header.clearBit(blockNum);
        //and reduce totalblocks count
        header.allocatedBlocksCount--;
        //also update header in file
        saveHeader();
    }
    
    // Read/Write blocks
    void read(int32_t blockNum, char* buffer) {
        readBlock(blockNum, buffer);
    }
    
    void write(int32_t blockNum, const char* buffer) {
        writeBlock(blockNum, buffer);
    }
    
    // Root block management (for B-tree index files)
    int32_t getRootBlock() const {
        return header.rootBlockIndex;
    }
    
    void setRootBlock(int32_t blockNum) {
        header.rootBlockIndex = blockNum;
        saveHeader();
    }
    
    // Record count management (for data files)
    int32_t getRecordCount() const {
        return header.recordCount;
    }
    
    void incrementRecordCount() {
        header.recordCount++;
        saveHeader();
    }
    
    void decrementRecordCount() {
        header.recordCount--;
        saveHeader();
    }
    
    void printInfo() const {
        std::cout << "\n=== " << filename << " ===" << std::endl;
        std::cout << "Signature: " << header.signature << std::endl;
        std::cout << "Total allocated blocks: " << header.allocatedBlocksCount << std::endl;
        std::cout << "Root block: " << header.rootBlockIndex << std::endl;
        std::cout << "Records: " << header.recordCount << std::endl;
        
        int allocated = 0;
        for (int i = 0; i < MAX_BLOCKS; i++) {
            if (header.isBitSet(i)) allocated++;
        }
        std::cout << "Allocated blocks: " << allocated << "/" << MAX_BLOCKS << std::endl;
    }
};

#endif