// #ifndef CV_HANDLER_H
// #define CV_HANDLER_H

// #include "../database/CVDB.hpp"
// #include <cstdint>
// #include <string>

// class CVHandler {
// private:
//     CVDatabase* cvdb;

// public:
//     CVHandler(CVDatabase* database) : cvdb(database) {}

//     // Submit new CV
//     int32_t submitCV(int32_t userId, const std::string& name, const std::string& email,
//                      const std::string& skills, int32_t experience, const std::string& position,
//                      const std::string& education, const std::string& location) {
//         return cvdb->addCV(userId, name, email, skills, experience, position, education, location);
//     }

//     // Get user's CV
//     CVRecord* getUserCV(int32_t userId) {
//         return cvdb->getCVByUserId(userId);
//     }

//     // Update existing CV
//     bool updateCV(int32_t userId, const std::string& name, const std::string& email,
//                   const std::string& skills, int32_t experience, const std::string& position,
//                   const std::string& education, const std::string& location) {
//         // Delete old CV
//         cvdb->deleteCVByUserId(userId);
        
//         // Add new CV
//         int32_t cvId = cvdb->addCV(userId, name, email, skills, experience, position, education, location);
//         return cvId > 0;
//     }

//     // Delete user's CV
//     bool deleteUserCV(int32_t userId) {
//         return cvdb->deleteCVByUserId(userId);
//     }
// };

// #endif