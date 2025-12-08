// #ifndef AUTH_HANDLER_H
// #define AUTH_HANDLER_H

// #include <string>
// #include <cstdint>

// class AuthHandler {
// public:
//     // Validate signup input
//     static bool validateSignup(const std::string& username, const std::string& password, const std::string& email) {
//         // Check username length
//         if (username.length() < 3 || username.length() > 50) {
//             return false;
//         }

//         // Check password strength (minimum 5 characters)
//         if (password.length() < 5) {
//             return false;
//         }

//         // Check email format (basic)
//         if (email.find('@') == std::string::npos || email.find('.') == std::string::npos) {
//             return false;
//         }

//         return true;
//     }

//     // Validate login input
//     static bool validateLogin(const std::string& username, const std::string& password) {
//         if (username.empty() || password.empty()) {
//             return false;
//         }
//         return true;
//     }

//     // Validate CV data
//     static bool validateCVData(const std::string& name, const std::string& email,
//                                const std::string& skills, int experience,
//                                const std::string& position, const std::string& education,
//                                const std::string& location) {
//         if (name.empty() || email. empty() || skills.empty() || position.empty() ||
//             education.empty() || location.empty()) {
//             return false;
//         }

//         if (experience < 0 || experience > 70) {
//             return false;
//         }

//         return true;
//     }
// };

// #endif