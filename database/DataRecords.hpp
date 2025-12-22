#ifndef RECORDS_H
#define RECORDS_H

#include <cstdint>
#include <cstring>
#include <iostream>

//for user data
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

//for cv records
struct CVRecord {
    int32_t cvId;
    int32_t userId;  //foreign key
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

//data BLOCK header
//har block k start me hoga ye header in data files
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





// Company/Job Posting Record
struct CompanyRecord {
    int32_t companyId;
    char companyName[100];
    char jobTitle[100];
    char requiredSkills[500];    // e.g., "C++,Python,SQL"
    int32_t minExperience;
    char position[100];          // e.g., "Senior Software Engineer"
    char education[100];         // e.g., "BS Computer Science"
    char location[50];
    int32_t salary;              // Monthly salary
    char description[500];
    bool isActive;               // Job still open? 
    int64_t postedDate;          // Unix timestamp
    bool isDeleted;
    
    CompanyRecord() : companyId(-1), minExperience(0), salary(0), 
                     isActive(true), postedDate(0), isDeleted(false) {
        memset(companyName, 0, sizeof(companyName));
        memset(jobTitle, 0, sizeof(jobTitle));
        memset(requiredSkills, 0, sizeof(requiredSkills));
        memset(position, 0, sizeof(position));
        memset(education, 0, sizeof(education));
        memset(location, 0, sizeof(location));
        memset(description, 0, sizeof(description));
    }
    
    static size_t size() { return sizeof(CompanyRecord); }
    
    void print() const {
        if (isDeleted) {
            std::cout << "[DELETED] Company ID: " << companyId << std::endl;
            return;
        }
        std::cout << "Company ID:  " << companyId << std:: endl;
        std::cout << "Company:  " << companyName << std:: endl;
        std::cout << "Job Title: " << jobTitle << std::endl;
        std::cout << "Skills Required: " << requiredSkills << std::endl;
        std:: cout << "Min Experience: " << minExperience << " years" << std::endl;
        std::cout << "Position: " << position << std:: endl;
        std::cout << "Salary: $" << salary << std::endl;
        std::cout << "Location: " << location << std::endl;
        std::cout << "Active: " << (isActive ? "Yes" : "No") << std::endl;
        std::cout << "---" << std::endl;
    }
};

// Application Record - Links a CV to a Company
struct ApplicationRecord {
    int32_t applicationId;
    int32_t userId;
    int32_t cvId;
    int32_t companyId;
    int64_t appliedDate;         // Unix timestamp
    char status[20];             // "pending", "reviewed", "accepted", "rejected"
    float matchScore;            // Ranking score (0.0 - 1.0)
    bool isDeleted;
    
    ApplicationRecord() : applicationId(-1), userId(-1), cvId(-1), 
                         companyId(-1), appliedDate(0), matchScore(0.0f), isDeleted(false) {
        memset(status, 0, sizeof(status));
        strncpy(status, "pending", sizeof(status) - 1);
    }
    
    static size_t size() { return sizeof(ApplicationRecord); }
    
    void print() const {
        if (isDeleted) {
            std::cout << "[DELETED] Application ID:  " << applicationId << std::endl;
            return;
        }
        std::cout << "Application ID: " << applicationId << std::endl;
        std::cout << "User ID: " << userId << std::endl;
        std::cout << "CV ID: " << cvId << std::endl;
        std::cout << "Company ID: " << companyId << std::endl;
        std:: cout << "Status: " << status << std::endl;
        std::cout << "Match Score: " << (matchScore * 100) << "%" << std::endl;
        std::cout << "---" << std::endl;
    }
};


#endif