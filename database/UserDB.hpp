#pragma once
#include "BlockManager.hpp"
#include "Btree.hpp"
#include "DataRecords.hpp"
#include <vector>
#include <string>

class UserDatabase {
private:
    BlockManager* indexMgr;
    BlockManager* dataMgr;
    BTree* btree;
    int32_t nextUserId;
    int32_t currentDataBlock;
    
    std::string hashPassword(const std::string& password) {
        std::hash<std::string> hasher;
        size_t hash = hasher(password + "salt_secret");
        char hashStr[65];
        snprintf(hashStr, sizeof(hashStr), "%016zx", hash);
        return std::string(hashStr);
    }
    
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
    UserDatabase(const std::string& indexFile, const std::string& dataFile) 
        : nextUserId(1), currentDataBlock(-1) {
        
        //aik block manager banao index file k liye
        indexMgr = new BlockManager(indexFile);
        //aik block manager banao data file k liye
        dataMgr = new BlockManager(dataFile);
        //ab b tree banao index file k uper
        btree = new BTree(indexMgr);
        
        // Find highest user ID
        std::vector<UserRecord> existing = getAllUsers();
        for (const auto& user : existing) {
            if (!user.isDeleted && user.userId >= nextUserId) {
                nextUserId = user.userId + 1;
            }
        }
        
        // Find or create first data block
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
                
                if (header.nextBlock == -1) break;
                currentDataBlock = header.nextBlock;
            }
        }
        
        std::cout << "UserDB ready. Next ID: " << nextUserId << std::endl;
    }
    
    ~UserDatabase() {
        delete btree;       
        delete indexMgr;
        delete dataMgr;
    }
    
    int32_t registerUser(const std::string& username, const std::string& password,
                        const std::string& email) {
        
        UserRecord user;
        user.userId = nextUserId++;
        user.timestamp = time(nullptr);
        user.isDeleted = false;
        
        strncpy(user.username, username.c_str(), sizeof(user.username) - 1);
        strncpy(user.email, email.c_str(), sizeof(user.email) - 1);
        
        std::string passHash = hashPassword(password);
        strncpy(user.passwordHash, passHash.c_str(), sizeof(user.passwordHash) - 1);
        
        // Read current block
        char buffer[BLOCK_SIZE];
        dataMgr->read(currentDataBlock, buffer);
        
        DataBlockHeader header;
        memcpy(&header, buffer, DataBlockHeader::size());
        
        // Check if record fits
        if (header.freeSpace < UserRecord::size()) {
            // Need new block
            int32_t newBlock = allocateDataBlock();
            header.nextBlock = newBlock;
            memcpy(buffer, &header, DataBlockHeader::size());
            dataMgr->write(currentDataBlock, buffer);
            
            currentDataBlock = newBlock;
            dataMgr->read(currentDataBlock, buffer);
            memcpy(&header, buffer, DataBlockHeader::size());
        }
        
        // Calculate offset
        int32_t offset = DataBlockHeader::size() + (header.recordCount * UserRecord::size());
        
        // Write record
        memcpy(buffer + offset, &user, UserRecord::size());
        
        // Update header
        header.recordCount++;
        header.freeSpace -= UserRecord::size();
        memcpy(buffer, &header, DataBlockHeader::size());
        
        dataMgr->write(currentDataBlock, buffer);
        dataMgr->incrementRecordCount();
        
        // Add to B-tree
        IndexEntry entry(user.userId, currentDataBlock, offset);
        btree->insert(entry);
        
        std::cout << "User " << user.userId << " registered" << std::endl;
        return user.userId;
    }
    
    UserRecord* getUserById(int32_t userId) {
        IndexEntry entry;
        if (btree->search(userId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            UserRecord* user = new UserRecord();
            memcpy(user, buffer + entry.offset, UserRecord::size());
            
            if (user->isDeleted) {
                delete user;
                return nullptr;
            }
            
            return user;
        }
        return nullptr;
    }
    
    bool deleteUser(int32_t userId) {
        IndexEntry entry;
        if (btree->search(userId, entry)) {
            // Mark as deleted in data file
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry.blockNum, buffer);
            
            UserRecord user;
            memcpy(&user, buffer + entry.offset, UserRecord::size());
            user.isDeleted = true;
            
            memcpy(buffer + entry.offset, &user, UserRecord::size());
            dataMgr->write(entry.blockNum, buffer);
            dataMgr->decrementRecordCount();
            
            // Remove from B-tree
            btree->remove(userId);
            
            std::cout << "User " << userId << " deleted" << std::endl;
            return true;
        }
        
        std::cout << "User " << userId << " not found" << std::endl;
        return false;
    }
    
    int32_t loginUser(const std::string& username, const std::string& password) {
        std::vector<UserRecord> users = getAllUsers();
        //password pehle hash function se guzarlo
        std::string passHash = hashPassword(password);
        
        for (const auto& user : users) {
            if (!user.isDeleted && 
                std::string(user.username) == username && 
                std::string(user.passwordHash) == passHash) {
                std::cout << "Login: " << username << std::endl;
                return user.userId;
            }
        }
        
        std::cout << "Login failed" << std::endl;
        return -1;
    }
    
    std::vector<UserRecord> getAllUsers() {
        std::vector<UserRecord> users;
        
        int32_t blockNum = dataMgr->getRootBlock();
        while (blockNum != -1) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(blockNum, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader::size());
            
            for (int i = 0; i < header.recordCount; i++) {
                int32_t offset = DataBlockHeader::size() + (i * UserRecord::size());
                UserRecord user;
                memcpy(&user, buffer + offset, UserRecord::size());
                
                if (!user.isDeleted) {
                    users.push_back(user);
                }
            }
            
            blockNum = header.nextBlock;
        }
        
        return users;
    }
    
    void printAll() {
        std::cout << "\n=== All Users ===" << std::endl;
        std::vector<UserRecord> users = getAllUsers();
        for (const auto& user : users) {
            user.print();
        }
        std::cout << "Total: " << users.size() << " users" << std::endl;
    }
    
    void printBTree() {
        std::cout << "\n=== User B-tree Index ===" << std::endl;
        btree->print();
    }
    
    void readHeader() {
        
        std::cout << "Index file header:" << std::endl;
        indexMgr->readFileHeader();
        std::cout << "Data file header:" << std::endl;
        dataMgr->readFileHeader();
    }
    void printInfo() {
        indexMgr->printInfo();
        dataMgr->printInfo();
    }
};

