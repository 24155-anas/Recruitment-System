#ifndef CV_DATABASE_H
#define CV_DATABASE_H

//Block manager root block aur baki blocks ko manage krta hai
//then in every db clas, har block ha header update/read/write hota ha (DataBlockHeader)
// then actual records us k baad store hote hain

#include "BlockManager.hpp"
#include "Btree.hpp"
#include "DataRecords.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include "datastructures/AVLTree.hpp"
#include "CVMatcher.hpp"


using namespace std;

class CVDatabase {
private:
    BlockManager* indexMgr; //for b tree index file for cvs
    BlockManager* dataMgr; //for actual data file for cvs
    BTree* btree;

    AVLTree<CVRecord> cvTree; //in-memory avl tree for cvs
    unordered_map<int32_t, int32_t> userIdToCvId; //map user id to cv id for quick lookup , in my getcvbyuserid function

    int32_t nextCvId;   //id assign krne k liye, future me aik function banaon ga jo unique id
    int32_t currentDataBlock; //kis block me mai abhi data rakhwa raha hon
    
    //alloacat a new data block
    // then write initial header in it
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

    //cvs ko memory me load krdo
    void loadAllCvsInMemory() {
        int32_t blockNum = dataMgr->getRootBlock();
        int count = 0;
        
        while (blockNum != -1) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(blockNum, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader:: size());
            
            for (int i = 0; i < header.recordCount; i++) {
                int32_t offset = DataBlockHeader::size() + (i * CVRecord::size());
                CVRecord cv;
                memcpy(&cv, buffer + offset, CVRecord::size());
                
                if (!cv.isDeleted) {
                    //avl me insert krdo
                    cvTree.insert(cv);
                    //map me bhi rakhdo
                    userIdToCvId[cv. userId] = cv.cvId;
                    count++;
                }
            }
            
            blockNum = header. nextBlock;
        }
        
        cout << "[CVDB] Loaded " << count << " CVs into memory" << endl;
    }

   void writeCVToDisk(const CVRecord& cv) {
        IndexEntry entry;
        
        //check if b tree me pehle ha
        if (btree->search(cv.cvId, entry)) {
            //update existing
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            memcpy(buffer + entry.offset, &cv, CVRecord::size());
            dataMgr->write(entry. blockNum, buffer);
        } else {
            //add new 
            char buffer[BLOCK_SIZE];
            dataMgr->read(currentDataBlock, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader::size());
            
            // Check if fits in current block
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
            
            //insert to b tree aswell
            IndexEntry newEntry(cv.cvId, currentDataBlock, offset);
            btree->insert(newEntry);
        }
    }
    
public:
    CVDatabase(const std::string& indexFile, const std::string& dataFile) : nextCvId(1), currentDataBlock(-1) {
        
        indexMgr = new BlockManager(indexFile);
        dataMgr = new BlockManager(dataFile);
        btree = new BTree(indexMgr);
        
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

        loadAllCvsInMemory();
        
        //get and set nextcvid
        vector<CVRecord> allCVs = cvTree.getAllElements();
        if (!allCVs.empty()) {
            nextCvId = allCVs.back().cvId + 1;  // Last element has max cvId
        }
        
        cout << "CVDB ready. Next ID: " << nextCvId << std::endl;
    }
    
    ~CVDatabase() {
        delete btree;
        delete indexMgr;
        delete dataMgr;
    }
    
    //add a cv to cv data file
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
        strncpy(cv.education, education. c_str(), sizeof(cv.education) - 1);
        strncpy(cv.location, location.c_str(), sizeof(cv.location) - 1);
        
        //write to disk
        writeCVToDisk(cv);
        
        //add to avl and map
        cvTree.insert(cv);
        userIdToCvId[cv.userId] = cv.cvId;

        cout << "CV " << cv.cvId << " added for user " << userId << endl;
        return cv.cvId;
    }
    CVRecord* getCVById(int32_t cvId) {
        CVRecord searchKey;
        searchKey.cvId = cvId;
        
        CVRecord* result = cvTree.find(searchKey);
        
        if (result && !result->isDeleted) {
            return new CVRecord(*result);
        }
        
        return nullptr;
    }
    
    CVRecord* getCVByUserId(int32_t userId) {
        //map me dhoondo
        auto it = userIdToCvId.find(userId);
        
        if (it != userIdToCvId.end()) {
            return getCVById(it->second);
        }
        
        return nullptr;
    }
    
    bool deleteCV(int32_t cvId) {
        IndexEntry entry;
        if (btree->search(cvId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            CVRecord cv;
            memcpy(&cv, buffer + entry.offset, CVRecord::size());
            //just mark as deleted
            cv.isDeleted = true;
            
            memcpy(buffer + entry.offset, &cv, CVRecord::size());
            dataMgr->write(entry.blockNum, buffer);
            dataMgr->decrementRecordCount();
            
            //remove from b gtree
            btree->remove(cvId); //b tree me remove.add etc funcftions me helpwers use ha jo automatically
                        //index files me bhi changes kr dete hain

            //remove from avl and map
            cvTree.remove(cv);
            for (auto it = userIdToCvId.begin(); it != userIdToCvId.end(); ++it) {
                if (it->second == cvId) {
                    userIdToCvId.erase(it);
                    break;
                }
            }

            
            cout << "CV " << cvId << " deleted" << endl;
            return true;
        }
        
        cout << "CV " << cvId << " not found" << endl;
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
        // std::vector<CVRecord> cvs;
        
        // int32_t blockNum = dataMgr->getRootBlock();
        // while (blockNum != -1) {
        //     char buffer[BLOCK_SIZE];
        //     dataMgr->read(blockNum, buffer);
            
        //     DataBlockHeader header;
        //     memcpy(&header, buffer, DataBlockHeader::size());
            
        //     for (int i = 0; i < header.recordCount; i++) {
        //         int32_t offset = DataBlockHeader::size() + (i * CVRecord::size());
        //         CVRecord cv;
        //         memcpy(&cv, buffer + offset, CVRecord::size());
                
        //         if (!cv.isDeleted) {
        //             cvs.push_back(cv);
        //         }
        //     }
            
        //     blockNum = header.nextBlock;
        // }
        
        // return cvs;

        vector<CVRecord> allCVs = cvTree.getAllElements();
        vector<CVRecord> result;
        for (const auto& cv : allCVs) {
            if (!cv.isDeleted) {
                result.push_back(cv);
            }
        }
        
        return result;
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