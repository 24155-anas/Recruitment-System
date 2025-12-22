// // // main.cpp - Complete test of recruitment database system
// #include "database/UserDB.hpp"
// #include "database/CVDB.hpp"
// #include <iostream>
// void printSeparator(const std::string& title) {
//     std::cout << "\n╔════════════════════════════════════════════════════════╗" << std::endl;
//     std::cout << "║  " << title;
//     for (size_t i = title.length(); i < 52; i++) std::cout << " ";
//     std::cout << "║" << std::endl;
//     std::cout << "╚════════════════════════════════════════════════════════╝\n" << std::endl;
// }

// int main() {
//     printSeparator("RECRUITMENT DATABASE SYSTEM");
    
//     std::cout << "Architecture:" << std::endl;
//     std::cout << "- 4KB blocks for ALL files (index + data)" << std::endl;
//     std::cout << "- Bitmap-based block allocation" << std::endl;
//     std::cout << "- B-tree indexing with deletion support" << std::endl;
//     std::cout << "- Separate files for clean organization\n" << std::endl;
    
//     // Initialize databases
//     UserDatabase userDB("user_index.dat", "user_data.dat");
//     CVDatabase cvDB("cv_index.dat", "cv_data.dat");
    
//     // ========== TEST 1: Register Users ==========
//     printSeparator("TEST 1: Register Users");
    
//     int u1 = userDB.registerUser("ahmed", "pass123", "ahmed@test.com");
//     int u2 = userDB.registerUser("fatima", "pass456", "fatima@test.com");
//     int u3 = userDB.registerUser("usman", "pass789", "usman@test.com");
//     int u4 = userDB.registerUser("ayesha", "pass111", "ayesha@test.com");
//     int u5 = userDB.registerUser("bilal", "pass222", "bilal@test.com");
    
//     userDB.printAll();
    
//     // ========== TEST 2: Add CVs ==========
//     printSeparator("TEST 2: Add CVs");
    
//     cvDB.addCV(u1, "Ahmed Ali", "ahmed@test.com", "C++,Python,SQL", 5,
//                "Software Engineer", "BS CS", "Lahore");
    
//     cvDB.addCV(u2, "Fatima Khan", "fatima@test.com", "Java,Spring,Docker", 3,
//                "Backend Developer", "BS SE", "Karachi");
    
//     cvDB.addCV(u3, "Usman Malik", "usman@test.com", "JavaScript,React,Node.js", 4,
//                "Full Stack Developer", "BS CS", "Islamabad");
    
//     cvDB.addCV(u4, "Ayesha Shah", "ayesha@test.com", "Python,ML,TensorFlow", 6,
//                "ML Engineer", "MS DS", "Lahore");
    
//     cvDB.addCV(u5, "Bilal Ahmed", "bilal@test.com", "C++,Embedded,RTOS", 8,
//                "Embedded Engineer", "BS EE", "Karachi");
    
//     // for(int i=0;i<25;i++){
//     //     cvDB.addCV(u1, "Test User", "testuser@test.com", "C++,Python", i % 10,
//     //            "Developer", "BS CS", "Lahore");

//     // }
//     cvDB.printAll();
    
//     // ========== TEST 3: B-tree Structure ==========
//     printSeparator("TEST 3: B-tree Indexes");
    
//     userDB.printBTree();
//     cvDB.printBTree();
    
//     // ========== TEST 4: Search Operations ==========
//     printSeparator("TEST 4: Search by ID");
    
//     std::cout << "Searching for User ID 2:" << std::endl;
//     UserRecord* user = userDB.getUserById(2);
//     if (user) {
//         user->print();
//         delete user;
//     }
    
//     std::cout << "\nSearching for CV ID 3:" << std::endl;
//     CVRecord* cv = cvDB.getCVById(3);
//     if (cv) {
//         cv->print();
//         delete cv;
//     }
    
//     std::cout << "\nSearching for CV by User ID 1:" << std::endl;
//     cv = cvDB.getCVByUserId(1);
//     if (cv) {
//         cv->print();
//         delete cv;
//     }
    
//     // ========== TEST 5: Login ==========
//     printSeparator("TEST 5: User Login");
    
//     std::cout << "Login with correct credentials:" << std::endl;
//     userDB.loginUser("ahmed", "pass123");
    
//     std::cout << "\nLogin with wrong password:" << std::endl;
//     userDB.loginUser("ahmed", "wrongpass");
    
//     // ========== TEST 6: DELETE Operations ==========
//     printSeparator("TEST 6: DELETE User & CV");
    
//     std::cout << "Before deletion:" << std::endl;
//     userDB.printAll();
//     cvDB.printAll();
    
//     std::cout << "\nDeleting User 3 (Usman) and their CV..." << std::endl;
//     userDB.deleteUser(3);
//     cvDB.deleteCVByUserId(3);
    
//     std::cout << "\nAfter deletion:" << std::endl;
//     userDB.printAll();
//     cvDB.printAll();
    
//     std::cout << "\nB-tree after deletion:" << std::endl;
//     userDB.printBTree();
//     cvDB.printBTree();
    
//     // ========== TEST 7: Try to access deleted records ==========
//     printSeparator("TEST 7: Access Deleted Records");
    
//     std::cout << "Trying to get deleted User 3:" << std::endl;
//     user = userDB.getUserById(3);
//     if (user) {
//         user->print();
//         delete user;
//     } else {
//         std::cout << "✓ User 3 not found (correctly deleted)" << std::endl;
//     }
    
//     std::cout << "\nTrying to get deleted CV 3:" << std::endl;
//     cv = cvDB.getCVById(3);
//     if (cv) {
//         cv->print();
//         delete cv;
//     } else {
//         std::cout << "✓ CV 3 not found (correctly deleted)" << std::endl;
//     }
    
//     // ========== TEST 8: Add more records to test block allocation ==========
//     printSeparator("TEST 8: Multiple Blocks");
    
//     std::cout << "Adding 10 more users to demonstrate block allocation...\n" << std::endl;
    
//     for (int i = 6; i <= 15; i++) {
//         std::string username = "user" + std::to_string(i);
//         std::string email = username + "@test.com";
//         int userId = userDB.registerUser(username, "password", email);
        
//         std::string name = "User " + std::to_string(i);
//         cvDB.addCV(userId, name, email, "C++,Python", i % 10,
//                    "Developer", "BS CS", "Lahore");
//     }
    
//     // ========== TEST 9: File Information ==========
//     printSeparator("TEST 9: Block & File Information");
    
//     userDB.printInfo();
//     cvDB.printInfo();
    
//     // ========== TEST 10: Calculate Statistics ==========
//     printSeparator("TEST 10: Storage Statistics");
    
//     std::cout << "Record Sizes:" << std::endl;
//     std::cout << "  UserRecord: " << UserRecord::size() << " bytes" << std::endl;
//     std::cout << "  CVRecord: " << CVRecord::size() << " bytes" << std::endl;
//     std::cout << "  DataBlockHeader: " << DataBlockHeader::size() << " bytes\n" << std::endl;
    
//     int usersPerBlock = (BLOCK_SIZE - DataBlockHeader::size()) / UserRecord::size();
//     int cvsPerBlock = (BLOCK_SIZE - DataBlockHeader::size()) / CVRecord::size();
    
//     std::cout << "Records per 4KB Block:" << std::endl;
//     std::cout << "  Users: ~" << usersPerBlock << " per block" << std::endl;
//     std::cout << "  CVs: ~" << cvsPerBlock << " per block\n" << std::endl;
    
//     std::vector<UserRecord> allUsers = userDB.getAllUsers();
//     std::vector<CVRecord> allCVs = cvDB.getAllCVs();
    
//     std::cout << "Current Database:" << std::endl;
//     std::cout << "  Active Users: " << allUsers.size() << std::endl;
//     std::cout << "  Active CVs: " << allCVs.size() << std::endl;
    
//     std::cout<<"Current BTREES STATUS\n";
//     userDB.printBTree();
//     cvDB.printBTree();
//     std::cout << std::endl;

//     // ========== TEST 11: Persistence Check ==========
//     printSeparator("TEST 11: Restart & Persistence");
    
//     std::cout << "Simulating program restart...\n" << std::endl;
    
//     {
//         std::cout << "Creating new database instances..." << std::endl;
//         UserDatabase userDB2("user_index.dat", "user_data.dat");
//         CVDatabase cvDB2("cv_index.dat", "cv_data.dat");
        
//         std::cout << "\n✓ Databases loaded from disk!" << std::endl;
//         std::cout << "\nData after restart:" << std::endl;
//         userDB2.printAll();
//         cvDB2.printAll();
        
//         std::cout << "\nB-tree after restart:" << std::endl;
//         userDB2.printBTree();
//         cvDB2.printBTree();
        
//         std::cout << "\n✓ All data persisted correctly!" << std::endl;
//     }
    
//     // ========== SUMMARY ==========
//     printSeparator("SUMMARY");
    
//     std::cout << "All tests completed successfully." << std::endl;
    
//     userDB.readHeader();
    
//     printSeparator("TEST COMPLETE");
    
//     return 0;
// }


#include <iostream>
#include "database/ApplicationDB.hpp"
#include "database/CompanyDB.hpp"
#include "database/CVDB.hpp"
#include "database/UserDB.hpp"

int main() {
    std::cout << "\n=== Testing Multi-Company Recruitment System ===\n" << std::endl;
    
    // Initialize databases
    UserDatabase userDB("test_user_index.dat", "test_user_data. dat");
    CVDatabase cvDB("test_cv_index.dat", "test_cv_data.dat");
    CompanyDatabase companyDB("test_company_index.dat", "test_company_data.dat");
    ApplicationDatabase appDB("test_app_index.dat", "test_app_data.dat");
    
    // 1. Register users
    std::cout << "\n--- Step 1: Register Users ---" << std::endl;
    int32_t user1 = userDB.registerUser("john_doe", "pass123", "john@email.com");
    int32_t user2 = userDB.registerUser("jane_smith", "pass456", "jane@email.com");
    
    // 2. Add companies
    std::cout << "\n--- Step 2: Add Companies ---" << std::endl;
    int32_t google = companyDB.addCompany(
        "Google", 
        "Senior Software Engineer",
        "C++,Python,SQL",
        5,
        "Senior Engineer",
        "BS Computer Science",
        "California",
        8000,
        "Work on cutting-edge technology"
    );
    
    int32_t microsoft = companyDB.addCompany(
        "Microsoft",
        "Backend Developer",
        "C#,. NET,Azure",
        3,
        "Backend Developer",
        "BS Computer Science",
        "Washington",
        7000,
        "Build cloud solutions"
    );
    
    // 3. Add CVs
    std::cout << "\n--- Step 3: Upload CVs ---" << std::endl;
    int32_t cv1 = cvDB.addCV(user1, "John Doe", "john@email.com", 
                             "C++,Python,Java,SQL", 6, 
                             "Software Engineer", "BS Computer Science", "California");
    
    int32_t cv2 = cvDB.addCV(user2, "Jane Smith", "jane@email.com", 
                             "C#,. NET,Azure,SQL", 4, 
                             "Backend Developer", "MS Computer Science", "Washington");
    
    // 4. Apply to companies
    std::cout << "\n--- Step 4: Users Apply to Jobs ---" << std::endl;
    
    // John applies to Google (good match)
    int32_t app1 = appDB.addApplication(user1, cv1, google, 0.92f);
    
    // John also applies to Microsoft (poor match)
    int32_t app2 = appDB.addApplication(user1, cv1, microsoft, 0.45f);
    
    // Jane applies to Microsoft (excellent match)
    int32_t app3 = appDB.addApplication(user2, cv2, microsoft, 0.95f);
    
    // Jane applies to Google (average match)
    int32_t app4 = appDB.addApplication(user2, cv2, google, 0.60f);
    
    // 5. View applications for Google
    std::cout << "\n--- Step 5: Admin Views CVs for Google ---" << std::endl;
    std::vector<ApplicationRecord> googleApps = appDB. getApplicationsByCompany(google);
    
    std::cout << "Applications to Google (sorted by score):" << std::endl;
    for (const auto& app : googleApps) {
        CVRecord* cv = cvDB.getCVById(app.cvId);
        if (cv) {
            std:: cout << "  Rank: #" << (&app - &googleApps[0] + 1) 
                      << " | " << cv->name 
                      << " | Score: " << (app.matchScore * 100) << "%" 
                      << " | Status: " << app.status << std::endl;
            delete cv;
        }
    }
    
    // 6. View applications for Microsoft
    std::cout << "\n--- Step 6: Admin Views CVs for Microsoft ---" << std::endl;
    std::vector<ApplicationRecord> msApps = appDB.getApplicationsByCompany(microsoft);
    
    std::cout << "Applications to Microsoft (sorted by score):" << std::endl;
    for (const auto& app : msApps) {
        CVRecord* cv = cvDB.getCVById(app.cvId);
        if (cv) {
            std::cout << "  Rank:  #" << (&app - &msApps[0] + 1) 
                      << " | " << cv->name 
                      << " | Score: " << (app.matchScore * 100) << "%" 
                      << " | Status: " << app.status << std::endl;
            delete cv;
        }
    }
    
    // 7. View John's applications
    std::cout << "\n--- Step 7: John Views His Applications ---" << std::endl;
    std::vector<ApplicationRecord> johnApps = appDB.getApplicationsByUser(user1);
    
    std::cout << "John's applications:" << std::endl;
    for (const auto& app : johnApps) {
        CompanyRecord* company = companyDB.getCompanyById(app. companyId);
        if (company) {
            std::cout << "  → " << company->companyName 
                      << " (" << company->jobTitle << ")" 
                      << " | Match:  " << (app.matchScore * 100) << "%" 
                      << " | Status: " << app.status << std::endl;
            delete company;
        }
    }
    
    // 8. Update application status
    std::cout << "\n--- Step 8: Admin Accepts Jane for Microsoft ---" << std::endl;
    appDB.updateStatus(app3, "accepted");
    
    // 9. Check duplicate application
    std::cout << "\n--- Step 9: Check Duplicate Application ---" << std::endl;
    bool alreadyApplied = appDB.hasUserAppliedToCompany(user1, google);
    std::cout << "Has John already applied to Google? " << (alreadyApplied ? "YES" : "NO") << std::endl;
    
    std::cout << "\n=== Test Complete!  ===\n" << std:: endl;
    
    return 0;
}
// }