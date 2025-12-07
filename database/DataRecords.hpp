#ifndef RECORDS_H
#define RECORDS_H

#include <cstdint>
#include <cstring>
#include <iostream>

// User Record
struct UserRecord {
    int32_t userId;
    char username[50];
    char passwordHash[65];
    char email[100];
    int64_t timestamp;
    bool isDeleted;  // Soft delete flag
    
    UserRecord() : userId(-1), timestamp(0), isDeleted(false) {
        memset(username, 0, sizeof(username));
        memset(passwordHash, 0, sizeof(passwordHash));
        memset(email, 0, sizeof(email));
    }
    
    static size_t size() { return sizeof(UserRecord); }
    
    void print() const {
        if (isDeleted) {
            std::cout << "[DELETED] User ID: " << userId << std::endl;
            return;
        }
        std::cout << "User ID: " << userId << std::endl;
        std::cout << "Username: " << username << std::endl;
        std::cout << "Email: " << email << std::endl;
        std::cout << "---" << std::endl;
    }
};

// CV Record
struct CVRecord {
    int32_t cvId;
    int32_t userId;  // Foreign key
    char name[50];
    char email[50];
    char skills[200];
    int32_t experience;
    char lastPosition[50];
    char education[100];
    char location[50];
    bool isDeleted;  // Soft delete flag
    
    CVRecord() : cvId(-1), userId(-1), experience(0), isDeleted(false) {
        memset(name, 0, sizeof(name));
        memset(email, 0, sizeof(email));
        memset(skills, 0, sizeof(skills));
        memset(lastPosition, 0, sizeof(lastPosition));
        memset(education, 0, sizeof(education));
        memset(location, 0, sizeof(location));
    }
    
    static size_t size() { return sizeof(CVRecord); }
    
    void print() const {
        if (isDeleted) {
            std::cout << "[DELETED] CV ID: " << cvId << std::endl;
            return;
        }
        std::cout << "CV ID: " << cvId << std::endl;
        std::cout << "User ID: " << userId << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Skills: " << skills << std::endl;
        std::cout << "Experience: " << experience << " years" << std::endl;
        std::cout << "Position: " << lastPosition << std::endl;
        std::cout << "Education: " << education << std::endl;
        std::cout << "Location: " << location << std::endl;
        std::cout << "---" << std::endl;
    }
};

// Data Block Header - stored at start of each data block
struct DataBlockHeader {
    int32_t blockNumber;
    int32_t recordCount;
    int32_t freeSpace;
    int32_t nextBlock;  // For chaining
    
    DataBlockHeader() : blockNumber(-1), recordCount(0), 
                       freeSpace(BLOCK_SIZE - sizeof(DataBlockHeader)), 
                       nextBlock(-1) {}
    
    static size_t size() { return sizeof(DataBlockHeader); }
};

#endif