#pragma once
#include "BlockManager.hpp"
#include "Btree.hpp"
#include "DataRecords.hpp"
#include <vector>
#include <string>
#include <ctime>

class ApplicationDatabase {
private:
    BlockManager* indexMgr;
    BlockManager* dataMgr;
    BTree* btree;
    int32_t nextApplicationId;
    int32_t currentDataBlock;
    
    int32_t allocateDataBlock() {
        int32_t blockNum = dataMgr->allocateBlock();
        
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        
        DataBlockHeader header;
        header.blockNumber = blockNum;
        header.recordCount = 0;
        header.freeSpace = BLOCK_SIZE - DataBlockHeader::size();
        header.nextBlock = -1;
        
        memcpy(buffer, &header, DataBlockHeader:: size());
        dataMgr->write(blockNum, buffer);
        
        return blockNum;
    }
    
public: 
    ApplicationDatabase(const std:: string& indexFile, const std:: string& dataFile) 
        : nextApplicationId(1), currentDataBlock(-1) {
        
        indexMgr = new BlockManager(indexFile);
        dataMgr = new BlockManager(dataFile);
        btree = new BTree(indexMgr);
        
        std::vector<ApplicationRecord> existing = getAllApplications();
        for (const auto& app : existing) {
            if (!app.isDeleted && app.applicationId >= nextApplicationId) {
                nextApplicationId = app.applicationId + 1;
            }
        }
        
        if (dataMgr->getRootBlock() == -1) {
            currentDataBlock = allocateDataBlock();
            dataMgr->setRootBlock(currentDataBlock);
        } else {
            currentDataBlock = dataMgr->getRootBlock();
            char buffer[BLOCK_SIZE];
            
            while (true) {
                dataMgr->read(currentDataBlock, buffer);
                DataBlockHeader header;
                memcpy(&header, buffer, DataBlockHeader:: size());
                
                if (header.nextBlock == -1) break;
                currentDataBlock = header.nextBlock;
            }
        }
        
        std::cout << "ApplicationDB ready.  Next ID: " << nextApplicationId << std::endl;
    }
    
    ~ApplicationDatabase() {
        delete btree;
        delete indexMgr;
        delete dataMgr;
    }
    
    //adding an application
    int32_t addApplication(int32_t userId, int32_t cvId, int32_t companyId, float matchScore) {
        ApplicationRecord app;
        app.applicationId = nextApplicationId++;
        app.userId = userId; ///kis user ki ha 
        app.cvId = cvId;  //cv ki id
        app.companyId = companyId;  //kis company ki ha
        app.appliedDate = time(nullptr);
        app.matchScore = matchScore;
        app.isDeleted = false;
        strncpy(app.status, "pending", sizeof(app.status) - 1);
        
        //save to disk

        //below same logic as for userdb,cvdb,companydb
        char buffer[BLOCK_SIZE];
        dataMgr->read(currentDataBlock, buffer);
        
        DataBlockHeader header;
        memcpy(&header, buffer, DataBlockHeader::size());
        
        if (header.freeSpace < ApplicationRecord:: size()) {
            int32_t newBlock = allocateDataBlock();
            header.nextBlock = newBlock;
            memcpy(buffer, &header, DataBlockHeader::size());
            dataMgr->write(currentDataBlock, buffer);
            
            currentDataBlock = newBlock;
            dataMgr->read(currentDataBlock, buffer);
            memcpy(&header, buffer, DataBlockHeader::size());
        }
        
        int32_t offset = DataBlockHeader::size() + (header.recordCount * ApplicationRecord::size());
        memcpy(buffer + offset, &app, ApplicationRecord::size());
        
        header.recordCount++;
        header.freeSpace -= ApplicationRecord::size();
        memcpy(buffer, &header, DataBlockHeader::size());
        
        dataMgr->write(currentDataBlock, buffer);
        dataMgr->incrementRecordCount();
        
        IndexEntry entry(app.applicationId, currentDataBlock, offset);
        btree->insert(entry);
        
        std::cout << "Application " << app.applicationId << " added:  User " << userId 
                  << " → Company " << companyId << " (Score: " << (matchScore * 100) << "%)" << std::endl;
        return app.applicationId;
    }
    
    //get application by id
    ApplicationRecord* getApplicationById(int32_t applicationId) {
        IndexEntry entry;
        if (btree->search(applicationId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            ApplicationRecord* app = new ApplicationRecord();
            memcpy(app, buffer + entry.offset, ApplicationRecord::size());
            
            if (app->isDeleted) {
                delete app;
                return nullptr;
            }
            
            return app;
        }
        return nullptr;
    }
    
    //get all applications
    std::vector<ApplicationRecord> getAllApplications() {
        std::vector<ApplicationRecord> applications;
        int32_t blockNum = dataMgr->getRootBlock();
        
        while (blockNum != -1) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(blockNum, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader::size());
            
            for (int i = 0; i < header.recordCount; i++) {
                int32_t offset = DataBlockHeader::size() + (i * ApplicationRecord::size());
                ApplicationRecord app;
                memcpy(&app, buffer + offset, ApplicationRecord::size());
                
                if (! app.isDeleted) {
                    applications.push_back(app);
                }
            }
            
            blockNum = header.nextBlock;
        }
        
        return applications;
    }
    
    // Get all applications for a specific company 
    std::vector<ApplicationRecord> getApplicationsByCompany(int32_t companyId) {
        std::vector<ApplicationRecord> allApps = getAllApplications();
        std::vector<ApplicationRecord> companyApps;
        
        for (const auto& app : allApps) {
            if (app.companyId == companyId && !app.isDeleted) {
                companyApps.push_back(app);
            }
        }
        
        // Sort by match score (highest first)
        // std::sort(companyApps.begin(), companyApps.end(), 
        //     [](const ApplicationRecord& a, const ApplicationRecord& b) {
        //         return a.matchScore > b.matchScore;
        //     });
        
        return companyApps;
    }
    
    // Get all applications by a specific user
    std::vector<ApplicationRecord> getApplicationsByUser(int32_t userId) {
        std::vector<ApplicationRecord> allApps = getAllApplications();
        std::vector<ApplicationRecord> userApps;
        
        for (const auto& app :  allApps) {
            if (app.userId == userId && ! app.isDeleted) {
                userApps.push_back(app);
            }
        }
        
        // Sort by date (newest first)
        // std::sort(userApps.begin(), userApps.end(), 
        //     [](const ApplicationRecord& a, const ApplicationRecord& b) {
        //         return a.appliedDate > b.appliedDate;
        //     });
        
        return userApps;
    }
    
    // // Get all applications for a specific CV
    // std::vector<ApplicationRecord> getApplicationsByCV(int32_t cvId) {
    //     std::vector<ApplicationRecord> allApps = getAllApplications();
    //     std::vector<ApplicationRecord> cvApps;
        
    //     for (const auto& app : allApps) {
    //         if (app.cvId == cvId && !app.isDeleted) {
    //             cvApps.push_back(app);
    //         }
    //     }
        
    //     return cvApps;
    // }
    
    //check if user already applied to company
    bool hasUserAppliedToCompany(int32_t userId, int32_t companyId) {
        std::vector<ApplicationRecord> allApps = getAllApplications();
        
        for (const auto& app :  allApps) {
            if (app.userId == userId && app.companyId == companyId && !app.isDeleted) {
                return true;
            }
        }
        return false;
    }
    
    //update application status (pending → accepted/rejected/reviewed)
    bool updateStatus(int32_t applicationId, const std::string& newStatus) {
        IndexEntry entry;
        if (btree->search(applicationId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            ApplicationRecord app;
            memcpy(&app, buffer + entry. offset, ApplicationRecord::size());
            
            //update status
            strncpy(app.status, newStatus.c_str(), sizeof(app.status) - 1);
            
            //write back
            memcpy(buffer + entry.offset, &app, ApplicationRecord::size());
            dataMgr->write(entry. blockNum, buffer);
            
            std::cout << "Application " << applicationId << " status updated:   " << newStatus << std:: endl;
            return true;
        }
        return false;
    }
    
    //delete application (soft delete)
    bool deleteApplication(int32_t applicationId) {
        IndexEntry entry;
        if (btree->search(applicationId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            ApplicationRecord app;
            memcpy(&app, buffer + entry.offset, ApplicationRecord::size());
            app.isDeleted = true;
            
            memcpy(buffer + entry.offset, &app, ApplicationRecord::size());
            dataMgr->write(entry.blockNum, buffer);
            
            btree->remove(applicationId);
            
            std::cout << "Application " << applicationId << " deleted" << std::endl;
            return true;
        }
        return false;
    }
    
    // Get count of applications for a company
    int32_t countApplicationsByCompany(int32_t companyId) {
        return getApplicationsByCompany(companyId).size();
    }
    
    // Get count of applications by a user
    int32_t countApplicationsByUser(int32_t userId) {
        return getApplicationsByUser(userId).size();
    }
    
    //print all applications (for debugging)
    void printAll() {
        std::cout << "\n=== All Applications ===" << std::endl;
        std::vector<ApplicationRecord> apps = getAllApplications();
        for (const auto& app : apps) {
            app.print();
        }
        std::cout << "Total:  " << apps.size() << " applications" << std::endl;
    }
};

