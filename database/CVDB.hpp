#ifndef CV_DATABASE_H
#define CV_DATABASE_H

#include "BlockManager.hpp"
#include "Btree.hpp"
#include "DataRecords.hpp"
#include <vector>
#include <string>

class CVDatabase {
private:
    BlockManager* indexMgr;
    BlockManager* dataMgr;
    BTree* btree;

    int32_t nextCvId;   //id assign krne k liye, future me aik function banaon ga jo unique id
    int32_t currentDataBlock; //kis block me mai abhi data rakhwa raha hon
    
    int32_t allocateDataBlock() {
        int32_t blockNum = dataMgr->allocateBlock();
        
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        
        DataBlockHeader header;
        header.blockNumber = blockNum;
        header.recordCount = 0;
        header.freeSpace = BLOCK_SIZE - DataBlockHeader::size();
        header.nextBlock = -1;
        
        memcpy(buffer, &header, DataBlockHeader::size());
        dataMgr->write(blockNum, buffer);
        
        return blockNum;
    }
    
public:
    CVDatabase(const std::string& indexFile, const std::string& dataFile) 
        : nextCvId(1), currentDataBlock(-1) {
        
        indexMgr = new BlockManager(indexFile);
        dataMgr = new BlockManager(dataFile);
        btree = new BTree(indexMgr);
        
        std::vector<CVRecord> existing = getAllCVs();
        for (const auto& cv : existing) {
            if (!cv.isDeleted && cv.cvId >= nextCvId) {
                nextCvId = cv.cvId + 1;
            }
        }
        
        if (dataMgr->getRootBlock() == -1) {
            currentDataBlock = allocateDataBlock();
            dataMgr->setRootBlock(currentDataBlock);
        } else {
            // Find last block in chain
            currentDataBlock = dataMgr->getRootBlock();
            char buffer[BLOCK_SIZE];
            
            while (true) {
                dataMgr->read(currentDataBlock, buffer);
                DataBlockHeader header;
                memcpy(&header, buffer, DataBlockHeader::size());
                
                if (header.nextBlock == -1) {
                    break;
                }
                currentDataBlock = header.nextBlock;
            }
        }
        
        std::cout << "CVDB ready. Next ID: " << nextCvId << std::endl;
    }
    
    ~CVDatabase() {
        delete btree;
        delete indexMgr;
        delete dataMgr;
    }
    
    int32_t addCV(int32_t userId, const std::string& name, const std::string& email,
                 const std::string& skills, int32_t experience, 
                 const std::string& lastPosition, const std::string& education,
                 const std::string& location) {
        
        CVRecord cv;
        cv.cvId = nextCvId++;
        cv.userId = userId;
        cv.experience = experience;
        cv.isDeleted = false;
        
        strncpy(cv.name, name.c_str(), sizeof(cv.name) - 1);
        strncpy(cv.email, email.c_str(), sizeof(cv.email) - 1);
        strncpy(cv.skills, skills.c_str(), sizeof(cv.skills) - 1);
        strncpy(cv.lastPosition, lastPosition.c_str(), sizeof(cv.lastPosition) - 1);
        strncpy(cv.education, education.c_str(), sizeof(cv.education) - 1);
        strncpy(cv.location, location.c_str(), sizeof(cv.location) - 1);
        
        // Read current block
        char buffer[BLOCK_SIZE];
        dataMgr->read(currentDataBlock, buffer);
        
        DataBlockHeader header;
        memcpy(&header, buffer, DataBlockHeader::size());
        
        // Check if fits
        if (header.freeSpace < CVRecord::size()) {
            int32_t newBlock = allocateDataBlock();
            header.nextBlock = newBlock;
            memcpy(buffer, &header, DataBlockHeader::size());
            dataMgr->write(currentDataBlock, buffer);
            
            currentDataBlock = newBlock;
            dataMgr->read(currentDataBlock, buffer);
            memcpy(&header, buffer, DataBlockHeader::size());
        }
        
        int32_t offset = DataBlockHeader::size() + (header.recordCount * CVRecord::size());
        
        memcpy(buffer + offset, &cv, CVRecord::size());
        
        header.recordCount++;
        header.freeSpace -= CVRecord::size();
        memcpy(buffer, &header, DataBlockHeader::size());
        
        dataMgr->write(currentDataBlock, buffer);
        dataMgr->incrementRecordCount();
        
        
        IndexEntry entry(cv.cvId, currentDataBlock, offset);
        btree->insert(entry);
        
        std::cout << "✓ CV " << cv.cvId << " added for user " << userId << std::endl;
        return cv.cvId;
    }
    
    CVRecord* getCVById(int32_t cvId) {
        IndexEntry entry;
        if (btree->search(cvId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            CVRecord* cv = new CVRecord();
            memcpy(cv, buffer + entry.offset, CVRecord::size());
            
            if (cv->isDeleted) {
                delete cv;
                return nullptr;
            }
            
            return cv;
        }
        return nullptr;
    }
    
    CVRecord* getCVByUserId(int32_t userId) {
        std::vector<CVRecord> allCVs = getAllCVs();
        
        for (const auto& cv : allCVs) {
            if (cv.userId == userId) {
                CVRecord* result = new CVRecord(cv);
                return result;
            }
        }
        return nullptr;
    }
    
    bool deleteCV(int32_t cvId) {
        IndexEntry entry;
        if (btree->search(cvId, entry)) {
            // Mark as deleted in data file
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            CVRecord cv;
            memcpy(&cv, buffer + entry.offset, CVRecord::size());
            cv.isDeleted = true;
            
            memcpy(buffer + entry.offset, &cv, CVRecord::size());
            dataMgr->write(entry.blockNum, buffer);
            dataMgr->decrementRecordCount();
            
            // Remove from B-tree
            btree->remove(cvId);
            
            std::cout << "✓ CV " << cvId << " deleted" << std::endl;
            return true;
        }
        
        std::cout << "✗ CV " << cvId << " not found" << std::endl;
        return false;
    }
    
    bool deleteCVByUserId(int32_t userId) {
        CVRecord* cv = getCVByUserId(userId);
        if (cv) {
            int32_t cvId = cv->cvId;
            delete cv;  
            return deleteCV(cvId);
        }
        return false;
    }
    
    std::vector<CVRecord> getAllCVs() {
        std::vector<CVRecord> cvs;
        
        int32_t blockNum = dataMgr->getRootBlock();
        while (blockNum != -1) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(blockNum, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader::size());
            
            for (int i = 0; i < header.recordCount; i++) {
                int32_t offset = DataBlockHeader::size() + (i * CVRecord::size());
                CVRecord cv;
                memcpy(&cv, buffer + offset, CVRecord::size());
                
                if (!cv.isDeleted) {
                    cvs.push_back(cv);
                }
            }
            
            blockNum = header.nextBlock;
        }
        
        return cvs;
    }
    
    void printAll() {
        std::cout << "\n=== All CVs ===" << std::endl;
        std::vector<CVRecord> cvs = getAllCVs();
        for (const auto& cv : cvs) {
            cv.print();
        }
        std::cout << "Total: " << cvs.size() << " CVs" << std::endl;
    }
    
    void printBTree() {
        std::cout << "\n=== CV B-tree Index ===" << std::endl;
        btree->print();
    }
    
    void printInfo() {
        indexMgr->printInfo();
        dataMgr->printInfo();
    }
};

#endif