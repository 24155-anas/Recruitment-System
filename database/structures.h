#pragma once

#include <cstring>
#include <string>
#include <iostream>
using namespace std;


// User record structure
struct UserRecord {
    int32_t userId;
    char username[50];
    char email[100];
    char passwordHash[64];  // SHA-256 hash
    char fullName[100];
    
    UserRecord() : userId(-1) {
        memset(username, 0, sizeof(username));
        memset(email, 0, sizeof(email));
        memset(passwordHash, 0, sizeof(passwordHash));
        memset(fullName, 0, sizeof(fullName));
    }
    
    UserRecord(int32_t id, const char* uname, const char* em, 
               const char* pwdHash, const char* name) 
        : userId(id) {
        
        strncpy(username, uname, sizeof(username)-1);
        username[sizeof(username)-1] = '\0';
        
        strncpy(email, em, sizeof(email)-1);
        email[sizeof(email)-1] = '\0';
        
        strncpy(passwordHash, pwdHash, sizeof(passwordHash)-1);
        passwordHash[sizeof(passwordHash)-1] = '\0';
        
        strncpy(fullName, name, sizeof(fullName)-1);
        fullName[sizeof(fullName)-1] = '\0';
    }
};

// CV record structure (now includes userId)
struct CVRecord {
    int32_t cvId;
    int32_t userId;           // Link to user who owns this CV
    char name[50];
    char email[50];
    char skills[200];
    int experience;
    char lastPosition[50];

    
    CVRecord() : cvId(-1), userId(-1), experience(0) {
        memset(name, 0, sizeof(name));
        memset(email, 0, sizeof(email));
        memset(skills, 0, sizeof(skills));
        memset(lastPosition, 0, sizeof(lastPosition));

    }
    
    CVRecord(int32_t cvId_, int32_t userId_, const char* name_, const char* email_, 
             const char* skills_, int exp_, const char* lastPos_) 
        : cvId(cvId_), userId(userId_), experience(exp_) {
        
        strncpy(name, name_, sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        
        strncpy(email, email_, sizeof(email)-1);
        email[sizeof(email)-1] = '\0';
        
        strncpy(skills, skills_, sizeof(skills)-1);
        skills[sizeof(skills)-1] = '\0';
        
        strncpy(lastPosition, lastPos_, sizeof(lastPosition)-1);
        lastPosition[sizeof(lastPosition)-1] = '\0';

    }
    
    void print() const {
        std::cout << "CV ID: " << cvId << std::endl;
        std::cout << "User ID: " << userId << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Email: " << email << std::endl;
        std::cout << "Skills: " << skills << std::endl;
        std::cout << "Experience: " << experience << " years" << std::endl;
        std::cout << "Last Position: " << lastPosition << std::endl;

        std::cout << "------------------------" << std::endl;
    }
};

