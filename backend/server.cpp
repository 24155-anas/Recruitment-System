

// //add mutex

// #include "httplib.h"
// #include "json.hpp"
// #include "../database/UserDB.hpp"
// #include "../database/CVDB.hpp"
// #include "SessionManager.hpp"
// #include <fstream>
// #include <iostream>
// # include <filesystem>
// using json = nlohmann::json;
// using namespace httplib;

// // Global database instances
// UserDatabase* userdb = nullptr;
// CVDatabase* cvdb = nullptr;
// SessionManager* sessionMgr = nullptr;

// // Helper function to read files
// std::string read_file(const std::string& filename) {
//     std::string filepath = "../frontend/" + filename;
    
//     std::ifstream file(filepath);
//     if (!file.is_open()) {
//         std::cerr << "ERROR: File not found: " << filepath << std::endl;
//         return "";
//     }
//     std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     std::cout << "✓ Served: " << filepath << std::endl;
//     return content;
// }

// int main() {
//     std::cout << "\n=== Recruitment System Server ===" << std::endl;
//     std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    
//     // Initialize databases
//     std::cout << "Initializing databases..." << std::endl;
//     userdb = new UserDatabase("../database/user.idx", "../database/user.dat");
//     cvdb = new CVDatabase("../database/cv.idx", "../database/cv.dat");
//     sessionMgr = new SessionManager();

//     Server svr;

//     // Set logger
//     svr.set_logger([](const Request& req, const Response& res) {
//         std::cout << "REQUEST: " << req.method << " " << req.path << " -> " << res.status << std::endl;
//     });

//     // --- SERVE ROOT ---
//     svr.Get("/", [](const Request&, Response& res) {
//         std::string content = read_file("index.html");
//         if (!content.empty()) {
//             res.set_content(content, "text/html");
//         } else {
//             res.set_content("<h1>500 Error: index.html not found</h1>", "text/html");
//             res.status = 500;
//         }
//     });



//     // --- SERVE index.html EXPLICITLY ---
//     svr.Get("/index.html", [](const Request&, Response& res) {
//         std::cout << "GET /index.html called" << std::endl;
//         std::string content = read_file("index.html");
//         if (!content.empty()) {
//             std::cout << "Serving index. html (" << content.length() << " bytes)" << std::endl;
//             res.set_content(content, "text/html");
//         } else {
//             std::cout << "ERROR: Could not read index.html" << std::endl;
//             res.set_content("<h1>500 Error: index.html not found</h1>", "text/html");
//             res.status = 500;
//         }
//     });

//     // --- SERVE dashboard.html ---
//     svr.Get("/dashboard.html", [](const Request&, Response& res) {
//         std::cout << "GET /dashboard.html called" << std::endl;
//         std::string content = read_file("dashboard.html");
//         if (!content.empty()) {
//             std::cout << "Serving dashboard.html (" << content.length() << " bytes)" << std::endl;
//             res.set_content(content, "text/html");
//         } else {
//             std::cout << "ERROR: Could not read dashboard.html" << std::endl;
//             res.set_content("<h1>500 Error: dashboard.html not found</h1>", "text/html");
//             res. status = 500;
//         }
//     });

//     // --- SERVE cv-form.html ---
//     svr.Get("/cv-form.html", [](const Request&, Response& res) {
//         std::cout << "GET /cv-form. html called" << std::endl;
//         std::string content = read_file("cv-form.html");
//         if (!content.empty()) {
//             res.set_content(content, "text/html");
//         } else {
//             res.set_content("<h1>500 Error: cv-form. html not found</h1>", "text/html");
//             res.status = 500;
//         }
//     });

//     // --- SERVE styles.css ---
//     svr.Get("/styles.css", [](const Request&, Response& res) {
//         std::string content = read_file("styles.css");
//         if (!content. empty()) {
//             res.set_content(content, "text/css");
//         } else {
//             res.set_content("/* Error: styles.css not found */", "text/css");
//         }
//     });

//     // --- SIGNUP ENDPOINT ---
//     svr.Post("/signup", [](const Request& req, Response& res) {
//         try {
//             auto j = json::parse(req.body);
//             std::string username = j["username"];
//             std::string password = j["password"];
//             std::string email = j["email"];

//             std::cout << "[SIGNUP] Username: " << username << std::endl;

//             // Check for duplicate username
//             auto users = userdb->getAllUsers();
//             for (const auto& u : users) {
//                 if (std::string(u.username) == username) {
//                     res.set_content(
//                         json({{"success", false}, {"message", "Username already exists"}}).dump(),
//                         "application/json"
//                     );
//                     return;
//                 }
//             }

//             // Register user
//             int32_t userId = userdb->registerUser(username, password, email);
//             if (userId > 0) {
//                 std::string sessionId = sessionMgr->createSession(userId);
//                 res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly");
//                 res.set_content(json({{"success", true}}).dump(), "application/json");
//                 std::cout << "[SIGNUP] ✓ User registered: " << username << " (ID: " << userId << ")" << std::endl;
//             } else {
//                 res.set_content(
//                     json({{"success", false}, {"message", "Failed to register"}}).dump(),
//                     "application/json"
//                 );
//             }
//         } catch (std::exception& e) {
//             std::cerr << "[SIGNUP ERROR] " << e.what() << std::endl;
//             res. set_content(
//                 json({{"success", false}, {"message", "Invalid request"}}).dump(),
//                 "application/json"
//             );
//         }
//     });

//     // --- LOGIN ENDPOINT ---
//     svr.Post("/login", [](const Request& req, Response& res) {
//         try {
//             auto j = json::parse(req.body);
//             std::string username = j["username"];
//             std::string password = j["password"];

//             std::cout << "[LOGIN] Username: " << username << std::endl;

//             int32_t userId = userdb->loginUser(username, password);
//             if (userId > 0) {
//                 std::string sessionId = sessionMgr->createSession(userId);
//                 res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly");
//                 res.set_content(json({{"success", true}}).dump(), "application/json");
//                 std::cout << "[LOGIN] ✓ User logged in: " << username << " (ID: " << userId << ")" << std::endl;
//             } else {
//                 res.set_content(
//                     json({{"success", false}, {"message", "Invalid username or password"}}).dump(),
//                     "application/json"
//                 );
//                 std::cout << "[LOGIN] ✗ Failed login: " << username << std::endl;
//             }
//         } catch (std::exception& e) {
//             std::cerr << "[LOGIN ERROR] " << e.what() << std::endl;
//             res.set_content(
//                 json({{"success", false}, {"message", "Invalid request"}}).dump(),
//                 "application/json"
//             );
//         }
//     });

//     // --- LOGOUT ENDPOINT ---
//     svr.Post("/logout", [](const Request& req, Response& res) {
//         auto cookie = req.get_header_value("Cookie");
//         std::cout << "[LOGOUT] Cookie: " << cookie << std::endl;
//         sessionMgr->deleteSession(cookie);
//         res. set_content(json({{"success", true}}).dump(), "application/json");
//     });

//     // --- SUBMIT CV ENDPOINT ---
//     svr.Post("/submit_cv", [](const Request& req, Response& res) {
//         try {
//             auto cookie = req.get_header_value("Cookie");
//             int32_t userId = sessionMgr->getUserId(cookie);

//             if (userId == -1) {
//                 res.set_content(
//                     json({{"success", false}, {"message", "Not logged in"}}).dump(),
//                     "application/json"
//                 );
//                 return;
//             }

//             auto j = json::parse(req. body);
//             int32_t cvId = cvdb->addCV(
//                 userId,
//                 j["name"].get<std::string>(),
//                 j["email"].get<std::string>(),
//                 j["skills"].get<std::string>(),
//                 std::stoi(j["experience"]. get<std::string>()),
//                 j["lastPosition"].get<std::string>(),
//                 j["education"].get<std::string>(),
//                 j["location"].get<std::string>()
//             );

//             if (cvId > 0) {
//                 res.set_content(json({{"success", true}}).dump(), "application/json");
//                 std::cout << "[CV SUBMIT] ✓ User " << userId << ", CV ID: " << cvId << std::endl;
//             } else {
//                 res.set_content(
//                     json({{"success", false}, {"message", "Failed to submit CV"}}).dump(),
//                     "application/json"
//                 );
//             }
//         } catch (std::exception& e) {
//             std::cerr << "[CV ERROR] " << e.what() << std::endl;
//             res. set_content(
//                 json({{"success", false}, {"message", "Invalid request"}}).dump(),
//                 "application/json"
//             );
//         }
//     });

//     // --- GET CV ENDPOINT ---
//     svr.Get("/get_cv", [](const Request& req, Response& res) {
//         try {
//             auto cookie = req.get_header_value("Cookie");
//             int32_t userId = sessionMgr->getUserId(cookie);

//             if (userId == -1) {
//                 res.set_content(
//                     json({{"success", false}, {"message", "Not logged in"}}).dump(),
//                     "application/json"
//                 );
//                 return;
//             }

//             CVRecord* cvPtr = cvdb->getCVByUserId(userId);
//             if (cvPtr) {
//                 json jcv = {
//                     {"cvId", cvPtr->cvId},
//                     {"userId", cvPtr->userId},
//                     {"name", cvPtr->name},
//                     {"email", cvPtr->email},
//                     {"skills", cvPtr->skills},
//                     {"experience", cvPtr->experience},
//                     {"lastPosition", cvPtr->lastPosition},
//                     {"education", cvPtr->education},
//                     {"location", cvPtr->location}
//                 };
//                 delete cvPtr;
//                 res.set_content(
//                     json({{"success", true}, {"cv", jcv}}).dump(),
//                     "application/json"
//                 );
//             } else {
//                 res.set_content(
//                     json({{"success", false}, {"message", "No CV found"}}).dump(),
//                     "application/json"
//                 );
//             }
//         } catch (std::exception& e) {
//             std::cerr << "[GET CV ERROR] " << e.what() << std::endl;
//             res.set_content(
//                 json({{"success", false}, {"message", "Error retrieving CV"}}).dump(),
//                 "application/json"
//             );
//         }
//     });

//     // Start server
//     std::cout << "Server running on http://localhost:8080/" << std::endl;
//     std::cout << "Press Ctrl+C to stop\n" << std::endl;

//     if (! svr.listen("0.0.0.0", 8080)) {
//         std::cerr << "Failed to start server" << std::endl;
//         return 1;
//     }

//     // Cleanup
//     delete userdb;
//     delete cvdb;
//     delete sessionMgr;

//     return 0;
// }


// backend/server.cpp - API-only server with CORS support

#define CPPHTTPLIB_PAYLOAD_MAX_LENGTH (50 * 1024 * 1024)  // 50MB
#define CPPHTTPLIB_FORM_URL_ENCODED_PAYLOAD_MAX_LENGTH (50 * 1024 * 1024)
#define CPPHTTPLIB_READ_TIMEOUT_SECOND 60
#define CPPHTTPLIB_WRITE_TIMEOUT_SECOND 60
// #include "httplib.h"
#include "httplib.h"

#include "json.hpp"
#include "../database/UserDB.hpp"
#include "../database/CVDB.hpp"
#include "../database/CVMatcher.hpp"
#include "SessionManager.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>
#include "PDFParser.hpp"

using json = nlohmann::json;
using namespace httplib;
using namespace std;

// Global database instances
UserDatabase* userdb = nullptr;
CVDatabase* cvdb = nullptr;
SessionManager* sessionMgr = nullptr;

// Helper function to check if user is admin
bool isAdmin(int32_t userId) {
    UserRecord* user = userdb->getUserById(userId);
    if (user && std::string(user->username) == "admin") {
        delete user;
        return true;
    }
    if (user) delete user;
    return false;
}

int main() {
    std::cout << "\n=== Recruitment System API Server ===" << std::endl;
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    
    // Initialize databases
    std::cout << "Initializing databases..." << std::endl;
    userdb = new UserDatabase("user_index.dat", "user_data.dat");
    cvdb = new CVDatabase("cv_index.dat", "cv_data.dat");
    sessionMgr = new SessionManager();

    Server svr;
 svr.set_payload_max_length(20 * 1024 * 1024);  // 20MB
    // ========== CORS MIDDLEWARE ==========
    svr.set_pre_routing_handler([](const Request& req, Response& res) {
        // Allow requests from frontend server
        res.set_header("Access-Control-Allow-Origin", "http://localhost:3000"); // Change to your frontend URL
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Cookie");
        res.set_header("Access-Control-Allow-Credentials", "true");
        
        // Handle preflight OPTIONS request
        if (req.method == "OPTIONS") {
            res. status = 204;
            return Server::HandlerResponse::Handled;
        }
        return Server::HandlerResponse::Unhandled;
    });

    // Set logger
    svr.set_logger([](const Request& req, const Response& res) {
        std::cout << "REQUEST: " << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    // ========== API ENDPOINTS ==========

    // --- SIGNUP ENDPOINT ---
    svr.Post("/api/signup", [](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];
            std::string email = j["email"];

            std::cout << "[SIGNUP] Username: " << username << std::endl;

            // Check for duplicate username
            auto users = userdb->getAllUsers();
            for (const auto& u : users) {
                if (std::string(u.username) == username) {
                    res.set_content(
                        json({{"success", false}, {"message", "Username already exists"}}).dump(),
                        "application/json"
                    );
                    return;
                }
            }

            // Register user
            int32_t userId = userdb->registerUser(username, password, email);
            if (userId > 0) {
                std::string sessionId = sessionMgr->createSession(userId);
                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly; SameSite=None; Secure");
                res. set_content(json({{"success", true}}).dump(), "application/json");
                std::cout << "[SIGNUP] ✓ User registered: " << username << " (ID: " << userId << ")" << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Failed to register"}}).dump(),
                    "application/json"
                );
            }
        } catch (std::exception& e) {
            std::cerr << "[SIGNUP ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Invalid request"}}).dump(),
                "application/json"
            );
        }
    });

    // --- LOGIN ENDPOINT ---
    svr.Post("/api/login", [](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];

            std::cout << "[LOGIN] Username: " << username << std::endl;

            int32_t userId = userdb->loginUser(username, password);
            if (userId > 0) {
                std::string sessionId = sessionMgr->createSession(userId);
                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly; SameSite=None; Secure");
                
                // Check if admin
                bool admin = isAdmin(userId);
                
                res.set_content(
                    json({{"success", true}, {"isAdmin", admin}}).dump(), 
                    "application/json"
                );
                std::cout << "[LOGIN] ✓ User logged in: " << username << " (ID: " << userId << ", Admin: " << admin << ")" << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Invalid username or password"}}).dump(),
                    "application/json"
                );
                std::cout << "[LOGIN] ✗ Failed login: " << username << std::endl;
            }
        } catch (std::exception& e) {
            std::cerr << "[LOGIN ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Invalid request"}}).dump(),
                "application/json"
            );
        }
    });

    // --- LOGOUT ENDPOINT ---
    svr.Post("/api/logout", [](const Request& req, Response& res) {
        auto cookie = req.get_header_value("Cookie");
        std::cout << "[LOGOUT] Cookie: " << cookie << std::endl;
        sessionMgr->deleteSession(cookie);
        res.set_content(json({{"success", true}}).dump(), "application/json");
    });

    // --- SUBMIT CV ENDPOINT ---
    svr.Post("/api/submit_cv", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);

            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                return;
            }

            auto j = json::parse(req.body);
            int32_t cvId = cvdb->addCV(
                userId,
                j["name"]. get<std::string>(),
                j["email"].get<std::string>(),
                j["skills"].get<std::string>(),
                std::stoi(j["experience"]. get<std::string>()),
                j["lastPosition"].get<std::string>(),
                j["education"].get<std::string>(),
                j["location"]. get<std::string>()
            );

            if (cvId > 0) {
                res.set_content(json({{"success", true}}).dump(), "application/json");
                std::cout << "[CV SUBMIT] ✓ User " << userId << ", CV ID: " << cvId << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Failed to submit CV"}}).dump(),
                    "application/json"
                );
            }
        } catch (std::exception& e) {
            std::cerr << "[CV ERROR] " << e.what() << std::endl;
            res. set_content(
                json({{"success", false}, {"message", "Invalid request"}}).dump(),
                "application/json"
            );
        }
    });




//  // --- UPLOAD CV PDF ENDPOINT ---
// svr.Post("/api/upload_cv", [](const Request& req, Response& res) {
//     try {
//         auto cookie = req.get_header_value("Cookie");
//         int32_t userId = sessionMgr->getUserId(cookie);
        
//         if (userId == -1) {
//             res.set_content(
//                 json({{"success", false}, {"message", "Not logged in"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         std::cout << "[UPLOAD] Request received" << std::endl;
//         std::cout << "  Content-Type: " << req.get_header_value("Content-Type") << std::endl;
//         std::cout << "  Body size: " << req.body.length() << std::endl;
        
//         // YOUR httplib.h has has_file(), so let's check the Request structure
//         // The issue is:  has_file() is in MultipartFormData, not Request
        
//         // SOLUTION: Parse the body manually since we can't access the abstraction
        
//         std::string contentType = req.get_header_value("Content-Type");
        
//         // Verify it's multipart
//         if (contentType.find("multipart/form-data") == std::string::npos) {
//             res.set_content(
//                 json({{"success", false}, {"message", "Content-Type must be multipart/form-data"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Extract boundary
//         size_t boundaryPos = contentType.find("boundary=");
//         if (boundaryPos == std::string::npos) {
//             res.set_content(
//                 json({{"success", false}, {"message", "No boundary found"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        
//         // Parse the multipart body
//         std::string body = req.body;
        
//         // Find the cv field
//         size_t cvFieldPos = body.find("name=\"cv\"");
//         if (cvFieldPos == std::string:: npos) {
//             res.set_content(
//                 json({{"success", false}, {"message", "No 'cv' field found"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Find Content-Type for this field
//         size_t contentTypePos = body. find("Content-Type:", cvFieldPos);
//         if (contentTypePos == std::string::npos) {
//             res.set_content(
//                 json({{"success", false}, {"message", "No Content-Type for cv field"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Check if it's a PDF
//         size_t pdfCheck = body.find("application/pdf", contentTypePos);
//         if (pdfCheck == std::string::npos || pdfCheck > contentTypePos + 50) {
//             res.set_content(
//                 json({{"success", false}, {"message", "File must be a PDF"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Find start of file data (after blank line)
//         size_t dataStart = body.find("\r\n\r\n", contentTypePos);
//         if (dataStart == std::string::npos) {
//             dataStart = body.find("\n\n", contentTypePos);
//             dataStart = (dataStart != std::string::npos) ? dataStart + 2 : std::string::npos;
//         } else {
//             dataStart += 4;
//         }
        
//         if (dataStart == std::string::npos) {
//             res.set_content(
//                 json({{"success", false}, {"message", "Invalid multipart format"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Find end of file data (next boundary)
//         size_t dataEnd = body.find("\r\n" + boundary, dataStart);
//         if (dataEnd == std::string::npos) {
//             dataEnd = body.find("\n" + boundary, dataStart);
//             if (dataEnd == std::string:: npos) {
//                 res. set_content(
//                     json({{"success", false}, {"message", "Could not find end of file data"}}).dump(),
//                     "application/json"
//                 );
//                 return;
//             }
//         }
        
//         // Extract PDF content
//         std::string pdfContent = body.substr(dataStart, dataEnd - dataStart);
        
//         std::cout << "  Extracted PDF:  " << pdfContent.length() << " bytes" << std::endl;
        
//         // Validate PDF header
//         if (pdfContent. length() < 4 || pdfContent.substr(0, 4) != "%PDF") {
//             res.set_content(
//                 json({{"success", false}, {"message", "Invalid PDF file"}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Save temporarily
//         std::string tempPath = "../uploads/cv_" + std::to_string(userId) + "_" + 
//                                std::to_string(time(nullptr)) + ".pdf";
        
//         system("mkdir -p ../uploads");
        
//         std::ofstream ofs(tempPath, std::ios::binary);
//         ofs.write(pdfContent.c_str(), pdfContent.length());
//         ofs.close();
        
//         std::cout << "  Saved:  " << tempPath << std::endl;
        
//         // Parse PDF
//         ParsedCV parsed = parseCV(tempPath);
        
//         // Cleanup
//         std:: remove(tempPath.c_str());
        
//         if (!parsed.success) {
//             res.set_content(
//                 json({{"success", false}, {"message", parsed.errorMessage}}).dump(),
//                 "application/json"
//             );
//             return;
//         }
        
//         // Save to database
//         int32_t cvId = cvdb->addCV(
//             userId,
//             parsed.name,
//             parsed.email,
//             parsed. skills,
//             parsed.experience,
//             parsed.lastPosition,
//             parsed.education,
//             parsed.location
//         );
        
//         if (cvId > 0) {
//             res.set_content(
//                 json({
//                     {"success", true},
//                     {"cvId", cvId},
//                     {"parsed", {
//                         {"name", parsed.name},
//                         {"email", parsed.email},
//                         {"skills", parsed.skills},
//                         {"experience", parsed.experience},
//                         {"lastPosition", parsed.lastPosition},
//                         {"education", parsed.education},
//                         {"location", parsed.location}
//                     }}
//                 }).dump(),
//                 "application/json"
//             );
//             std::cout << "[UPLOAD] ✓ Success! CV ID: " << cvId << std::endl;
//         } else {
//             res. set_content(
//                 json({{"success", false}, {"message", "Failed to save"}}).dump(),
//                 "application/json"
//             );
//         }
        
//     } catch (std::exception& e) {
//         std::cerr << "[UPLOAD ERROR] " << e.what() << std::endl;
//         res.set_content(
//             json({{"success", false}, {"message", std::string(e.what())}}).dump(),
//             "application/json"
//         );
//     }
// });
// --- UPLOAD CV PDF ENDPOINT (Using multipart parser) ---
svr.Post("/api/upload_cv", [](const Request& req, Response& res) {
    auto cookie = req.get_header_value("Cookie");
    int32_t userId = sessionMgr->getUserId(cookie);
    
    if (userId == -1) {
        json response;
        response["success"] = false;
        response["message"] = "Not logged in";
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    std::cout << "[UPLOAD] User " << userId << std::endl;
    std::cout << "  Content-Length: " << req.get_header_value("Content-Length") << std::endl;
    std::cout << "  Content-Type: " << req.get_header_value("Content-Type") << std::endl;
    
    // Check if request has files parameter
    if (req.files.empty() && req.body.empty()) {
        json response;
        response["success"] = false;
        response["message"] = "No file data received.  Both req.files and req.body are empty. ";
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    std::string pdfContent;
    std::string filename;
    
    // Try req.files first (newer httplib)
    if (!req.files.empty()) {
        std::cout << "  Using req.files (newer httplib)" << std::endl;
        
        auto it = req.files.find("cv");
        if (it == req.files.end()) {
            json response;
            response["success"] = false;
            response["message"] = "No 'cv' file field found";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        pdfContent = it->second.content;
        filename = it->second.filename;
        
    } else if (!req.body.empty()) {
        // Fallback to manual parsing (older httplib)
        std::cout << "  Using req.body manual parsing (older httplib)" << std::endl;
        std::cout << "  Body size: " << req.body.length() << std::endl;
        
        std::string contentType = req.get_header_value("Content-Type");
        size_t boundaryPos = contentType.find("boundary=");
        
        if (boundaryPos == std::string::npos) {
            json response;
            response["success"] = false;
            response["message"] = "No boundary in Content-Type";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        std::string boundary = "--" + contentType.substr(boundaryPos + 9);
        const std::string& body = req. body;
        
        // Find cv field
        size_t cvPos = body.find("name=\"cv\"");
        if (cvPos == std::string::npos) {
            json response;
            response["success"] = false;
            response["message"] = "No 'cv' field in multipart data";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // Find filename
        size_t filenamePos = body.find("filename=\"", cvPos);
        if (filenamePos != std::string::npos && filenamePos < cvPos + 200) {
            size_t filenameStart = filenamePos + 10;
            size_t filenameEnd = body.find("\"", filenameStart);
            if (filenameEnd != std::string::npos) {
                filename = body.substr(filenameStart, filenameEnd - filenameStart);
            }
        }
        
        // Find data start
        size_t dataStart = body.find("\r\n\r\n", cvPos);
        if (dataStart != std::string::npos) {
            dataStart += 4;
        } else {
            dataStart = body.find("\n\n", cvPos);
            if (dataStart != std::string::npos) dataStart += 2;
        }
        
        if (dataStart == std:: string::npos) {
            json response;
            response["success"] = false;
            response["message"] = "Cannot find data start in multipart";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // Find data end
        size_t dataEnd = body.find("\r\n" + boundary, dataStart);
        if (dataEnd == std::string::npos) {
            dataEnd = body.find("\n" + boundary, dataStart);
        }
        
        if (dataEnd == std::string::npos) {
            json response;
            response["success"] = false;
            response["message"] = "Cannot find data end in multipart";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // Extract PDF
        pdfContent = body.substr(dataStart, dataEnd - dataStart);
    } else {
        json response;
        response["success"] = false;
        response["message"] = "No file data - both req.files and req.body are empty";
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    std:: cout << "  PDF size: " << pdfContent.length() << " bytes" << std:: endl;
    std::cout << "  Filename: " << filename << std::endl;
    
    // Validate PDF
    if (pdfContent.length() < 4 || pdfContent.substr(0, 4) != "%PDF") {
        std::cout << "  ERROR: Not a valid PDF.  First 20 bytes:" << std::endl << "    ";
        for (size_t i = 0; i < 20 && i < pdfContent.length(); i++) {
            printf("%02X ", (unsigned char)pdfContent[i]);
        }
        std:: cout << std::endl;
        
        json response;
        response["success"] = false;
        response["message"] = "Invalid PDF file - missing PDF header";
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    // Save file
    std::string tempPath = "uploads/cv_" + std::to_string(userId) + "_" + 
                           std::to_string(time(nullptr)) + ".pdf";
    
    std:: filesystem::create_directories("uploads");
    
    std::ofstream ofs(tempPath, std:: ios::binary);
    if (!ofs) {
        json response;
        response["success"] = false;
        response["message"] = "Cannot create file on server";
        res.set_content(response.dump(), "application/json");
        return;
    }
    
    ofs. write(pdfContent.c_str(), pdfContent.length());
    ofs.close();
    
    std::cout << "  ✓ Saved:  " << tempPath << std::endl;
    
    // Return success
    json response;
    response["success"] = true;
    response["message"] = "PDF uploaded successfully";
    response["filename"] = filename. empty() ? "uploaded.pdf" : filename;
    response["size"] = pdfContent.length();
    response["path"] = tempPath;
    
    res.set_content(response.dump(), "application/json");
    
    std::cout << "[UPLOAD] ✓ Success!" << std::endl;
});

    // // --- GET ALL CVs ENDPOINT (Admin only) ---
    // svr.Get("/api/cvs/all", [](const Request& req, Response& res) {
    //     try {
    //         auto cookie = req. get_header_value("Cookie");
    //         int32_t userId = sessionMgr->getUserId(cookie);
            
    //         // Check authentication
    //         if (userId == -1) {
    //             res.set_content(
    //                 json({{"success", false}, {"message", "Not logged in"}}).dump(),
    //                 "application/json"
    //             );
    //             return;
    //         }
            
    //         // Check admin role
    //         if (!isAdmin(userId)) {
    //             res.set_content(
    //                 json({{"success", false}, {"message", "Unauthorized - Admin only"}}).dump(),
    //                 "application/json"
    //             );
    //             std::cout << "[ADMIN] ✗ Unauthorized access attempt by User ID: " << userId << std::endl;
    //             return;
    //         }
            
    //         // Get all CVs from database
    //         std::vector<CVRecord> allCVs = cvdb->getAllCVs();
    //         json cvArray = json::array();
            
    //         for (const auto& cv : allCVs) {
    //             cvArray.push_back({
    //                 {"cvId", cv.cvId},
    //                 {"userId", cv.userId},
    //                 {"name", std::string(cv.name)},
    //                 {"email", std::string(cv.email)},
    //                 {"skills", std::string(cv.skills)},
    //                 {"experience", cv.experience},
    //                 {"lastPosition", std::string(cv.lastPosition)},
    //                 {"education", std::string(cv.education)},
    //                 {"location", std::string(cv.location)}
    //             });
    //         }
            
    //         res.set_content(
    //             json({{"success", true}, {"cvs", cvArray}}).dump(),
    //             "application/json"
    //         );
    //         std::cout << "[ADMIN] ✓ Served " << allCVs.size() << " CVs to admin" << std::endl;
            
    //     } catch (std::exception& e) {
    //         std::cerr << "[API ERROR] " << e.what() << std::endl;
    //         res.set_content(
    //             json({{"success", false}, {"message", "Server error"}}).dump(),
    //             "application/json"
    //         );
    //     }
    // });


    // --- GET ALL CVs ENDPOINT (Admin only) ---
    svr.Get("/api/cvs/all", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            // Check authentication
            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                return;
            }
            
            // Check admin role
            if (! isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Unauthorized - Admin only"}}).dump(),
                    "application/json"
                );
                std::cout << "[ADMIN] ✗ Unauthorized access attempt by User ID: " << userId << std:: endl;
                return;
            }
            
            // ========== NEW SCORING LOGIC ==========
            
            // Get job requirement from query parameters (or use defaults)
            std::string requiredSkills = req.get_param_value("skills");
            std::string minExpStr = req.get_param_value("experience");
            std::string education = req.get_param_value("education");
            std::string position = req.get_param_value("position");
            std::string location = req.get_param_value("location");
            
            // Default job requirement if not specified
            JobRequirement job;
            job.requiredSkills = requiredSkills.empty() ? "C++,Python" : requiredSkills;
            job.minExperience = minExpStr.empty() ? 3 : std::stoi(minExpStr);
            job.education = education.empty() ? "BS CS" : education;
            job. position = position.empty() ? "Software Engineer" : position;
            job.location = location.empty() ? "" : location;
            job.minAcceptableScore = 0.0; // Show all CVs
            
            // Get all CVs and score them
            std::vector<CVRecord> allCVs = cvdb->getAllCVs();
            std::vector<DetailedScore> rankedCVs = findBestCVs(allCVs, job, allCVs. size());
            
            // Convert to JSON with scores
            json cvArray = json::array();
            
            for (const auto& scoredCV : rankedCVs) {
                cvArray.push_back({
                    {"cvId", scoredCV. cv.cvId},
                    {"userId", scoredCV.cv.userId},
                    {"name", std::string(scoredCV.cv.name)},
                    {"email", std::string(scoredCV.cv.email)},
                    {"skills", std::string(scoredCV.cv.skills)},
                    {"experience", scoredCV.cv.experience},
                    {"lastPosition", std::string(scoredCV.cv.lastPosition)},
                    {"education", std::string(scoredCV.cv.education)},
                    {"location", std::string(scoredCV.cv.location)},
                    // Add scoring details
                    {"score", scoredCV.finalScore},
                    {"skillsScore", scoredCV.skillsScore},
                    {"experienceScore", scoredCV.experienceScore},
                    {"educationScore", scoredCV.educationScore},
                    {"positionScore", scoredCV.positionScore},
                    {"locationScore", scoredCV.locationScore}
                });
            }
            
            res.set_content(
                json({
                    {"success", true}, 
                    {"cvs", cvArray},
                    {"jobRequirement", {
                        {"skills", job.requiredSkills},
                        {"experience", job. minExperience},
                        {"education", job.education},
                        {"position", job.position},
                        {"location", job.location}
                    }}
                }).dump(),
                "application/json"
            );
            std::cout << "[ADMIN] ✓ Served " << rankedCVs.size() << " ranked CVs to admin" << std::endl;
            
        } catch (std::exception& e) {
            std::cerr << "[API ERROR] " << e.what() << std::endl;
            res. set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });





    
    // Start server
    std::cout << "\n=== API Server Started ===" << std::endl;
    std::cout << "Running on http://localhost:8080/" << std::endl;
    std::cout << "API Endpoints:" << std::endl;
    std::cout << "  POST /api/signup" << std::endl;
    std::cout << "  POST /api/login" << std::endl;
    std::cout << "  POST /api/logout" << std::endl;
    std::cout << "  POST /api/submit_cv" << std::endl;
    std::cout << "  GET  /api/get_cv" << std::endl;
    std::cout << "  GET  /api/cvs/all (Admin only)" << std::endl;
    std::cout << "\nPress Ctrl+C to stop\n" << std::endl;

    if (! svr.listen("0.0.0.0", 8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return 1;
    }

    // Cleanup
    delete userdb;
    delete cvdb;
    delete sessionMgr;

    return 0;
}