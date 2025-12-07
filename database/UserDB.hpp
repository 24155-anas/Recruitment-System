// #ifndef USERDB_H
// #define USERDB_H

// #include "Btree.hpp"
// #include <fstream>
// #include <iostream>
// #include <string>
// #include <functional>
// #include <cstring>

// struct UserRecord {
//     int32_t id;
//     char username[50];
//     char email[100];
//     char password[50];
//     char fullName[100];
    
//     UserRecord() : id(-1) {
//         memset(username, 0, sizeof(username));
//         memset(email, 0, sizeof(email));
//         memset(password, 0, sizeof(password));
//         memset(fullName, 0, sizeof(fullName));
//     }
    
//     UserRecord(int32_t id_, const char* uname, const char* em, 
//                const char* pwd, const char* name) 
//         : id(id_) {
//         strncpy(username, uname, sizeof(username)-1);
//         username[sizeof(username)-1] = '\0';
        
//         strncpy(email, em, sizeof(email)-1);
//         email[sizeof(email)-1] = '\0';
        
//         strncpy(password, pwd, sizeof(password)-1);
//         password[sizeof(password)-1] = '\0';
        
//         strncpy(fullName, name, sizeof(fullName)-1);
//         fullName[sizeof(fullName)-1] = '\0';
//     }
// };

// class UserDB {
// private:
//     BTree* indexTree;
//     std::fstream dataFile;
//     std::string dataFilename;
//     int32_t nextId;
    
// public:
//     UserDB(const std::string& indexFile, const std::string& dataFile)
//         : dataFilename(dataFile), nextId(1) {
        
//         indexTree = new BTree(indexFile);
        
//         this->dataFile.open(dataFilename, 
//             std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        
//         if (!this->dataFile.is_open()) {
//             this->dataFile.open(dataFilename, std::ios::out | std::ios::binary);
//             this->dataFile.close();
//             this->dataFile.open(dataFilename, 
//                 std::ios::in | std::ios::out | std::ios::binary);
//         }
        
//         // Find next ID
//         this->dataFile.seekg(0, std::ios::end);
//         std::streampos size = this->dataFile.tellg();
//         if (size > 0) {
//             nextId = (size / sizeof(UserRecord)) + 1;
//         }
//     }
    
//     ~UserDB() {
//         dataFile.close();
//         delete indexTree;
//     }
    
//     // Register new user
//     int32_t registerUser(const std::string& username, 
//                         const std::string& email,
//                         const std::string& password,
//                         const std::string& fullName) {
        
//         // Check if username exists
//         if (usernameExists(username)) {
//             std::cout << "Username already exists!" << std::endl;
//             return -1;
//         }
        
//         // Create user record
//         UserRecord user(nextId, username.c_str(), email.c_str(), 
//                        password.c_str(), fullName.c_str());
        
//         // Write to data file
//         dataFile.seekp(0, std::ios::end);
//         int64_t offset = dataFile.tellp();
        
//         dataFile.write(reinterpret_cast<const char*>(&user), sizeof(UserRecord));
//         dataFile.flush();
        
//         // Add to B-tree index
//         BTreeNode::IndexEntry entry(user.id, offset, sizeof(UserRecord));
//         indexTree->insert(entry);
        
//         std::cout << "Registered user ID " << user.id 
//                   << " with username " << username << std::endl;
        
//         return nextId++;
//     }
    
//     // Login user
//     UserRecord* loginUser(const std::string& username, const std::string& password) {
//         // Search all users for matching username
//         dataFile.seekg(0);
//         while (dataFile) {
//             UserRecord user;
//             dataFile.read(reinterpret_cast<char*>(&user), sizeof(UserRecord));
            
//             if (dataFile.gcount() == sizeof(UserRecord)) {
//                 if (std::string(user.username) == username) {
//                     // Check password
//                     if (std::string(user.password) == password) {
//                         UserRecord* foundUser = new UserRecord();
//                         *foundUser = user;
//                         return foundUser;
//                     } else {
//                         std::cout << "Invalid password!" << std::endl;
//                         return nullptr;
//                     }
//                 }
//             }
//         }
        
//         std::cout << "User not found!" << std::endl;
//         return nullptr;
//     }
    
//     // Get user by ID
//     UserRecord* getUserById(int32_t userId) {
//         BTreeNode::IndexEntry entry;
//         if (indexTree->search(userId, entry)) {
//             dataFile.seekg(entry.dataOffset);
//             UserRecord* user = new UserRecord();
//             dataFile.read(reinterpret_cast<char*>(user), sizeof(UserRecord));
//             return user;
//         }
//         return nullptr;
//     }
    
// private:
//     bool usernameExists(const std::string& username) {
//         dataFile.seekg(0);
//         while (dataFile) {
//             UserRecord user;
//             dataFile.read(reinterpret_cast<char*>(&user), sizeof(UserRecord));
            
//             if (dataFile.gcount() == sizeof(UserRecord)) {
//                 if (std::string(user.username) == username) {
//                     return true;
//                 }
//             }
//         }
//         return false;
//     }
// };

// #endif




#ifndef USER_DATABASE_H
#define USER_DATABASE_H

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
        
        indexMgr = new BlockManager(indexFile);
        dataMgr = new BlockManager(dataFile);
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
        delete btree;       // Delete B-tree first (uses indexMgr)
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
        
        std::cout << "✓ User " << user.userId << " registered" << std::endl;
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
            
            std::cout << "✓ User " << userId << " deleted" << std::endl;
            return true;
        }
        
        std::cout << "✗ User " << userId << " not found" << std::endl;
        return false;
    }
    
    int32_t loginUser(const std::string& username, const std::string& password) {
        std::vector<UserRecord> users = getAllUsers();
        std::string passHash = hashPassword(password);
        
        for (const auto& user : users) {
            if (!user.isDeleted && 
                std::string(user.username) == username && 
                std::string(user.passwordHash) == passHash) {
                std::cout << "✓ Login: " << username << std::endl;
                return user.userId;
            }
        }
        
        std::cout << "✗ Login failed" << std::endl;
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

#endif