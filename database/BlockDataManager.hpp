// #ifndef BLOCK_DATA_MANAGER_H
// #define BLOCK_DATA_MANAGER_H

// #include <fstream>
// #include <iostream>
// #include <cstring>
// #include <vector>
// #include <cstdint>


// const int DATA_BLOCK_SIZE = 4096;  // 4KB blocks for data
// const int MAX_DATA_BLOCKS = 1000;  // Maximum blocks in data file
// const int DATA_BITMAP_SIZE = MAX_DATA_BLOCKS / 8;

// // ============= FILE HEADER (stored in Block 0) =============
// struct DataFileHeader {
//     char signature[8];        // "CVDB" signature
//     int32_t totalBlocks;      // Total blocks allocated
//     int32_t firstDataBlock;   // First block for data
//     int32_t recordCount;      // Total records in file
//     uint8_t bitmap[DATA_BITMAP_SIZE];  // Bitmap for block allocation
    
//     DataFileHeader() : totalBlocks(1), firstDataBlock(1), recordCount(0) {
//         memset(signature, 0, sizeof(signature));
//         strncpy(signature, "CVDB", 4);
//         memset(bitmap, 0, DATA_BITMAP_SIZE);
//         setBit(0);  // Block 0 is header, always allocated
//     }
    
//     bool isBitSet(int blockNum) const {
//         int byteIndex = blockNum / 8;
//         int bitIndex = blockNum % 8;
//         return (bitmap[byteIndex] & (1 << bitIndex)) != 0;
//     }
    
//     void setBit(int blockNum) {
//         int byteIndex = blockNum / 8;
//         int bitIndex = blockNum % 8;
//         bitmap[byteIndex] |= (1 << bitIndex);
//     }
    
//     void clearBit(int blockNum) {
//         int byteIndex = blockNum / 8;
//         int bitIndex = blockNum % 8;
//         bitmap[byteIndex] &= ~(1 << bitIndex);
//     }
    
//     int findFreeBlock() const {
//         for (int i = 1; i < MAX_DATA_BLOCKS; i++) {  // Start from 1 (0 is header)
//             if (!isBitSet(i)) {
//                 return i;
//             }
//         }
//         return -1;  // No free blocks
//     }
    
//     void print() const {
//         std::cout << "Data File Header:" << std::endl;
//         std::cout << "  Signature: " << signature << std::endl;
//         std::cout << "  Total Blocks: " << totalBlocks << std::endl;
//         std::cout << "  Record Count: " << recordCount << std::endl;
//         std::cout << "  Bitmap: ";
//         int allocatedCount = 0;
//         for (int i = 0; i < MAX_DATA_BLOCKS; i++) {
//             if (isBitSet(i)) allocatedCount++;
//         }
//         std::cout << allocatedCount << "/" << MAX_DATA_BLOCKS << " blocks allocated" << std::endl;
//     }
// };

// // ============= BLOCK HEADER (stored at start of each data block) =============
// struct DataBlockHeader {
//     int32_t blockNumber;
//     int32_t recordCount;     // How many records in this block
//     int32_t nextBlock;       // Link to next block (-1 if last)
//     int32_t freeSpace;       // Bytes available in block
    
//     DataBlockHeader() : blockNumber(-1), recordCount(0), nextBlock(-1), 
//                        freeSpace(DATA_BLOCK_SIZE - sizeof(DataBlockHeader)) {}
    
//     static size_t size() { return sizeof(DataBlockHeader); }
// };

// // ============= BLOCK DATA MANAGER =============
// class BlockDataManager {
// private:
//     std::fstream dataFile;
//     std::string filename;
//     DataFileHeader fileHeader;
//     int32_t currentBlock;  // Last block being used for inserts
    
// public:
//     void readBlock(int32_t blockNum, char* buffer) {
//         dataFile.seekg(blockNum * DATA_BLOCK_SIZE, std::ios::beg);
//         dataFile.read(buffer, DATA_BLOCK_SIZE);
//     }
    
//     void writeBlock(int32_t blockNum, const char* buffer) {
//         dataFile.seekp(blockNum * DATA_BLOCK_SIZE, std::ios::beg);
//         dataFile.write(buffer, DATA_BLOCK_SIZE);
//         dataFile.flush();
//     }
    
//     void readFileHeader() {
//         char buffer[DATA_BLOCK_SIZE];
//         readBlock(0, buffer);
//         memcpy(&fileHeader, buffer, sizeof(DataFileHeader));
//     }
    
//     void writeFileHeader() {
//         char buffer[DATA_BLOCK_SIZE];
//         memset(buffer, 0, DATA_BLOCK_SIZE);
//         memcpy(buffer, &fileHeader, sizeof(DataFileHeader));
//         writeBlock(0, buffer);
//     }
    
//     BlockDataManager(const std::string& fname) : filename(fname), currentBlock(1) {
//         dataFile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        
//         if (!dataFile.is_open()) {
//             std::cout << "Creating new data file: " << filename << std::endl;
            
//             dataFile.open(filename, std::ios::out | std::ios::binary);
//             dataFile.close();
//             dataFile.open(filename, std::ios::in | std::ios::out | std::ios::binary);
            
//             fileHeader = DataFileHeader();
//             writeFileHeader();
            
//             allocateNewBlock();
//         } else {
//             readFileHeader();
            
//             if (strncmp(fileHeader.signature, "CVDB", 4) != 0) {
//                 std::cout << "Warning: Invalid data file signature" << std::endl;
//             }
            
//             for (int i = MAX_DATA_BLOCKS - 1; i >= 1; i--) {
//                 if (fileHeader.isBitSet(i)) {
//                     currentBlock = i;
//                     break;
//                 }
//             }
            
//             std::cout << "Loaded data file: " << filename << std::endl;
//         }
//     }
    
//     ~BlockDataManager() {
//         if (dataFile.is_open()) {
//             writeFileHeader();
//             dataFile.close();
//         }
//     }
    
//     // Allocate a new block
//     int32_t allocateNewBlock() {
//         int32_t freeBlock = fileHeader.findFreeBlock();
        
//         if (freeBlock == -1) {
//             std::cout << "Error: No free blocks available!" << std::endl;
//             return -1;
//         }
        
//         fileHeader.setBit(freeBlock);
//         fileHeader.totalBlocks++;
//         writeFileHeader();
        
//         char buffer[DATA_BLOCK_SIZE];
//         memset(buffer, 0, DATA_BLOCK_SIZE);
        
//         DataBlockHeader header;
//         header.blockNumber = freeBlock;
//         header.recordCount = 0;
//         header.nextBlock = -1;
//         header.freeSpace = DATA_BLOCK_SIZE - DataBlockHeader::size();
        
//         memcpy(buffer, &header, DataBlockHeader::size());
//         writeBlock(freeBlock, buffer);
        
//         currentBlock = freeBlock;
//         return freeBlock;
//     }
    
//     // Insert record (C++11 compatible - no structured bindings)
//     template<typename RecordType>
//     bool insertRecord(const RecordType& record, int32_t& blockNum, int32_t& offset) {
//         char buffer[DATA_BLOCK_SIZE];
//         readBlock(currentBlock, buffer);
        
//         DataBlockHeader header;
//         memcpy(&header, buffer, DataBlockHeader::size());
        
//         if (header.freeSpace < RecordType::size()) {
//             std::cout << "Block " << currentBlock << " full, allocating new block..." << std::endl;
            
//             int32_t newBlock = allocateNewBlock();
//             if (newBlock == -1) return false;
            
//             header.nextBlock = newBlock;
//             memcpy(buffer, &header, DataBlockHeader::size());
//             writeBlock(currentBlock, buffer);
            
//             currentBlock = newBlock;
//             readBlock(currentBlock, buffer);
//             memcpy(&header, buffer, DataBlockHeader::size());
//         }
        
//         offset = DataBlockHeader::size() + (header.recordCount * RecordType::size());
//         memcpy(buffer + offset, &record, RecordType::size());
        
//         header.recordCount++;
//         header.freeSpace -= RecordType::size();
//         memcpy(buffer, &header, DataBlockHeader::size());
        
//         writeBlock(currentBlock, buffer);
        
//         fileHeader.recordCount++;
//         writeFileHeader();
        
//         blockNum = currentBlock;
//         return true;
//     }
    
//     // Read record
//     template<typename RecordType>
//     RecordType readRecord(int32_t blockNum, int32_t offset) {
//         char buffer[DATA_BLOCK_SIZE];
//         readBlock(blockNum, buffer);
        
//         RecordType record;
//         memcpy(&record, buffer + offset, RecordType::size());
        
//         return record;
//     }
    
//     // Read all records
//     template<typename RecordType>
//     std::vector<RecordType> readAllRecords() {
//         std::vector<RecordType> records;
        
//         for (int32_t blockNum = 1; blockNum < MAX_DATA_BLOCKS; blockNum++) {
//             if (!fileHeader.isBitSet(blockNum)) continue;
            
//             char buffer[DATA_BLOCK_SIZE];
//             readBlock(blockNum, buffer);
            
//             DataBlockHeader header;
//             memcpy(&header, buffer, DataBlockHeader::size());
            
//             for (int32_t i = 0; i < header.recordCount; i++) {
//                 int32_t offset = DataBlockHeader::size() + (i * RecordType::size());
//                 RecordType record;
//                 memcpy(&record, buffer + offset, RecordType::size());
//                 records.push_back(record);
//             }
//         }
        
//         return records;
//     }
    
//     void printBlockInfo() {
//         std::cout << "\n=== Block Information ===" << std::endl;
//         fileHeader.print();
//         std::cout << "Block size: " << DATA_BLOCK_SIZE << " bytes (4 KB)" << std::endl;
        
//         int allocatedBlocks = 0;
//         for (int32_t i = 1; i < MAX_DATA_BLOCKS; i++) {
//             if (!fileHeader.isBitSet(i)) continue;
            
//             allocatedBlocks++;
//             char buffer[DATA_BLOCK_SIZE];
//             readBlock(i, buffer);
            
//             DataBlockHeader header;
//             memcpy(&header, buffer, DataBlockHeader::size());
            
//             std::cout << "\nBlock " << i << ":" << std::endl;
//             std::cout << "  Records: " << header.recordCount << std::endl;
//             std::cout << "  Free space: " << header.freeSpace << " bytes" << std::endl;
//             std::cout << "  Next block: " << header.nextBlock << std::endl;
//         }
//         std::cout << "\nTotal data blocks: " << allocatedBlocks << std::endl;
//     }
// };

// #endif