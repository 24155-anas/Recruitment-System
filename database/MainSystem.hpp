// #ifndef RECRUITMENT_SYSTEM_H
// #define RECRUITMENT_SYSTEM_H

// #include "UserDB.hpp"
// #include "CandidateDB.hpp"
// #include <iostream>
// #include <vector>

// class RecruitmentSystem {
// private:
//     UserDatabase* userDB;
//     CVDatabase* cvDB;
//     UserRecord* currentUser;
//     bool isLoggedIn;
    
// public:
//     RecruitmentSystem() : currentUser(nullptr), isLoggedIn(false) {
//         std::cout << "Initializing Recruitment System..." << std::endl;
//         userDB = new UserDatabase("user_index.dat", "users_data.dat");
//         cvDB = new CVDatabase("cv_index.dat", "cvs_data.dat");
//     }
    
//     ~RecruitmentSystem() {
//         delete userDB;
//         delete cvDB;
//         if (currentUser) delete currentUser;
//     }
    
//     // User management
//     bool registerUser(const std::string& username, const std::string& email,
//                      const std::string& password, const std::string& fullName) {
//         std::cout << "Registering user '" << username << "'..." << std::endl;
//         int32_t userId = userDB->registerUser(username, password, email);
//         return userId != -1;
//     }
    
//     bool login(const std::string& username, const std::string& password) {
//         std::cout << "Attempting login for '" << username << "'..." << std::endl;
        
//         int32_t userId = userDB->loginUser(username, password);
        
//         if (userId != -1) {
//             currentUser = userDB->getUserById(userId);
//             isLoggedIn = true;
//             std::cout << "Login successful for user ID " << userId << std::endl;
//             return true;
//         } else {
//             std::cout << "Login failed for '" << username << "'" << std::endl;
//             return false;
//         }
//     }
    
//     void logout() {
//         if (currentUser) {
//             std::cout << "Logging out user " << currentUser->username << std::endl;
//             delete currentUser;
//             currentUser = nullptr;
//             isLoggedIn = false;
//         }
//     }
    
//     // CV management
//     int32_t addCV(const std::string& name, const std::string& email,
//                  const std::string& skills, int experience, 
//                  const std::string& lastPosition, const std::string& education = "",
//                  const std::string& location = "") {
//         if (!isLoggedIn || !currentUser) {
//             std::cout << "Please login first!" << std::endl;
//             return -1;
//         }
        
//         std::cout << "Adding CV for user ID " << currentUser->userId << std::endl;
//         return cvDB->addCV(currentUser->userId, name, email, skills, experience, 
//                           lastPosition, education, location);
//     }
    
//     std::vector<CVRecord> getUserCVs() {
//         if (!isLoggedIn || !currentUser) {
//             std::cout << "Please login first!" << std::endl;
//             return {};
//         }
        
//         std::cout << "Getting CVs for user ID " << currentUser->userId << std::endl;
//         return cvDB->getCVsByUserId(currentUser->userId);
//     }
    
//     // Search functionality
//     std::vector<CVRecord> searchCVsBySkill(const std::string& skill) {
//         std::cout << "Searching CVs for skill '" << skill << "'..." << std::endl;
//         return cvDB->searchBySkill(skill);
//     }
    
//     std::vector<CVRecord> findBestMatches(const std::string& requiredSkills, 
//                                          int minExperience) {
//         std::cout << "Finding best matches for skills '" << requiredSkills 
//                   << "' with min " << minExperience << " years experience..." << std::endl;
        
//         auto allMatches = cvDB->searchBySkill(requiredSkills);
//         std::vector<CVRecord> filteredMatches;
        
//         for (const auto& cv : allMatches) {
//             if (cv.experience >= minExperience) {
//                 filteredMatches.push_back(cv);
//             }
//         }
        
//         return filteredMatches;
//     }
    
//     // Debug functions
//     void printAllUsers() {
//         userDB->printAllUsers();
//     }
    
//     void printAllCVs() {
//         cvDB->printAllCVs();
//     }
    
//     void printDatabaseInfo() {
//         std::cout << "\n=== Database Information ===" << std::endl;
//         userDB->printBlockInfo();
//         cvDB->printBlockInfo();
//     }
    
//     UserRecord* getCurrentUser() { 
//         return currentUser; 
//     }
    
//     bool isUserLoggedIn() { 
//         return isLoggedIn; 
//     }


//     void readBTree(){
//         userDB->printIndex();
//         cvDB->printIndex();

//     }
// };

// #endif

