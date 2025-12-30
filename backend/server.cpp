

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
#include "../database/CompanyDB.hpp"
#include "../database/ApplicationDB.hpp"


using json = nlohmann::json;
using namespace httplib;
using namespace std;

// Global database instances
// Global database instances
UserDatabase* userdb = nullptr;
CVDatabase* cvdb = nullptr;
CompanyDatabase* companydb = nullptr;      // NEW
ApplicationDatabase* appdb = nullptr;       // NEW
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
    companydb = new CompanyDatabase("company_index.dat", "company_data.dat");  // NEW
    appdb = new ApplicationDatabase("app_index.dat", "app_data.dat");          // NEW
    sessionMgr = new SessionManager();

    Server svr;
 svr.set_payload_max_length(20 * 1024 * 1024);  // 20MB
    // ========== CORS MIDDLEWARE ==========
    svr.set_pre_routing_handler([](const Request& req, Response& res) {
        // Allow requests from frontend server
        res.set_header("Access-Control-Allow-Origin", "*");
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
                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax");
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

                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly; SameSite=Lax");
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
// --- UPLOAD CV PDF ENDPOINT (Local parsing only) ---
svr.Post("/api/upload_cv", [](const Request& req, Response& res) {
    try {
        auto cookie = req.get_header_value("Cookie");
        int32_t userId = sessionMgr->getUserId(cookie);
        
        if (userId == -1) {
            json response;
            response["success"] = false;
            response["message"] = "Not logged in";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        std::cout << "[UPLOAD] User " << userId << std:: endl;
        
        std:: string pdfContent;
        std::string filename;
        
        // Get file from req.files
        if (!req. files.empty()) {
            std::cout << "  Using req. files (newer httplib)" << std::endl;
            
            auto it = req.files.find("cv");
            if (it == req.files.end()) {
                json response;
                response["success"] = false;
                response["message"] = "No 'cv' file field found";
                res.set_content(response. dump(), "application/json");
                return;
            }
            
            pdfContent = it->second.content;
            filename = it->second.filename;
            
        } else {
            json response;
            response["success"] = false;
            response["message"] = "No file data";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        std::cout << "  PDF size: " << pdfContent.length() << " bytes" << std::endl;
        std::cout << "  Filename:  " << filename << std::endl;
        
        // Validate PDF
        if (pdfContent. length() < 4 || pdfContent.substr(0, 4) != "%PDF") {
            json response;
            response["success"] = false;
            response["message"] = "Invalid PDF file";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // Save temporarily
        std::string tempPath = "uploads/cv_" + std::to_string(userId) + "_" + 
                               std::to_string(time(nullptr)) + ".pdf";
        
        std::filesystem::create_directories("uploads");
        
        std::ofstream ofs(tempPath, std::ios::binary);
        if (!ofs) {
            json response;
            response["success"] = false;
            response["message"] = "Cannot create temp file";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        ofs. write(pdfContent.c_str(), pdfContent.length());
        ofs.close();
        
        std::cout << "  ✓ Saved:  " << tempPath << std::endl;
        
        // ========== PARSE PDF ==========
        std::cout << "  Parsing PDF..." << std::endl;
        
        ParsedCV parsed;
        try {
            parsed = parseCV(tempPath);
        } catch (std::exception& e) {
            std::cerr << "  ✗ Parse exception: " << e.what() << std::endl;
            std::remove(tempPath.c_str());
            json response;
            response["success"] = false;
            response["message"] = std::string("Parse error: ") + e.what();
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        // Clean up temp file
        std:: remove(tempPath.c_str());
        
        if (! parsed.success) {
            std::cout << "  ✗ Parse failed:  " << parsed.errorMessage << std::endl;
            json response;
            response["success"] = false;
            response["message"] = "Failed to parse PDF: " + parsed.errorMessage;
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        std::cout << "  ✓ Parsed successfully:" << std::endl;
        std:: cout << "    Name:  '" << parsed.name << "'" << std::endl;
        std::cout << "    Email:  '" << parsed.email << "'" << std::endl;
        std::cout << "    Skills:  '" << parsed.skills << "'" << std::endl;
        std::cout << "    Experience:  " << parsed.experience << " years" << std::endl;
        std::cout << "    Position:  '" << parsed.lastPosition << "'" << std::endl;
        std::cout << "    Education: '" << parsed.education << "'" << std::endl;
        std::cout << "    Location: '" << parsed.location << "'" << std::endl;
        
        // ========== SAVE TO DATABASE ==========
        std::cout << "  Saving to database..." << std::endl;
        
        int32_t cvId = 0;
        try {
            cvId = cvdb->addCV(
                userId,
                parsed.name,
                parsed.email,
                parsed. skills,
                parsed.experience,
                parsed.lastPosition,
                parsed.education,
                parsed.location
            );
        } catch (std::exception& e) {
            std::cerr << "  ✗ Database exception: " << e.what() << std::endl;
            json response;
            response["success"] = false;
            response["message"] = std::string("Database error:  ") + e.what();
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        if (cvId <= 0) {
            std:: cout << "  ✗ Database save failed (returned ID: " << cvId << ")" << std::endl;
            json response;
            response["success"] = false;
            response["message"] = "Failed to save CV to database";
            res.set_content(response.dump(), "application/json");
            return;
        }
        
        std::cout << "  ✓ Saved to database with CV ID: " << cvId << std::endl;
        
        // Return success with parsed data
        json response;
        response["success"] = true;
        response["message"] = "CV uploaded and parsed successfully";
        response["cvId"] = cvId;
        
        json parsedData;
        parsedData["name"] = parsed.name;
        parsedData["email"] = parsed.email;
        parsedData["skills"] = parsed.skills;
        parsedData["experience"] = parsed.experience;
        parsedData["lastPosition"] = parsed.lastPosition;
        parsedData["education"] = parsed.education;
        parsedData["location"] = parsed.location;
        
        response["parsed"] = parsedData;
        
        res.set_content(response.dump(), "application/json");
        
        std::cout << "[UPLOAD] ✓ Complete success!" << std::endl;
        
    } catch (std::exception& e) {
        std::cerr << "[UPLOAD ERROR] Unhandled exception: " << e.what() << std::endl;
        
        try {
            json response;
            response["success"] = false;
            response["message"] = std::string("Server error: ") + e.what();
            res.set_content(response.dump(), "application/json");
        } catch (...) {
            res.set_content("{\"success\": false,\"message\":\"Fatal server error\"}", "application/json");
        }
        res.status = 500;
    } catch (...) {
        std::cerr << "[UPLOAD ERROR] Unknown exception" << std::endl;
        res.set_content("{\"success\":false,\"message\": \"Unknown server error\"}", "application/json");
        res.status = 500;
    }
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

        // --- GET USER'S CV ---
    svr.Get("/api/get_cv", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                res.status = 401;
                return;
            }
            
            std::cout << "[GET CV] User " << userId << " requesting CV" << std::endl;
            
            // Get user's CV
            CVRecord* cvPtr = cvdb->getCVByUserId(userId);
            
            if (cvPtr) {
                json cvData = {
                    {"cvId", cvPtr->cvId},
                    {"userId", cvPtr->userId},
                    {"name", std:: string(cvPtr->name)},
                    {"email", std:: string(cvPtr->email)},
                    {"skills", std:: string(cvPtr->skills)},
                    {"experience", cvPtr->experience},
                    {"lastPosition", std::string(cvPtr->lastPosition)},
                    {"education", std::string(cvPtr->education)},
                    {"location", std::string(cvPtr->location)}
                };
                
                delete cvPtr;
                
                res.set_content(
                    json({{"success", true}, {"cv", cvData}}).dump(),  // ← FIXED! 
                    "application/json"
                );
                
                std::cout << "[GET CV] ✓ CV found for user " << userId << std:: endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "No CV found"}}).dump(),
                    "application/json"
                );
                res.status = 404;
                std::cout << "[GET CV] ✗ No CV found for user " << userId << std::endl;
            }
            
        } catch (std::exception& e) {
            std::cerr << "[GET CV ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
            res.status = 500;
        }
    });

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


    //deletion
        // --- DELETE CV ENDPOINT (Admin only) ---
    svr.Delete("/api/cv/:id", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            // Check if logged in
            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                res.status = 401;
                return;
            }
            
            // Check if admin
            if (!isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Unauthorized - Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                std::cout << "[DELETE CV] ✗ Unauthorized attempt by User ID: " << userId << std:: endl;
                return;
            }
            
            // Get CV ID from URL parameter
            std::string cvIdStr = req.path_params.at("id");
            int32_t cvId = std::stoi(cvIdStr);
            
            std::cout << "[DELETE CV] Admin (User " << userId << ") deleting CV " << cvId << std::endl;
            
            // Delete the CV
            bool deleted = cvdb->deleteCV(cvId);
            
            if (deleted) {
                res.set_content(
                    json({{"success", true}, {"message", "CV deleted successfully"}, {"cvId", cvId}}).dump(),
                    "application/json"
                );
                std::cout << "[DELETE CV] ✓ CV " << cvId << " deleted by admin" << std::endl;
            } else {
                res. set_content(
                    json({{"success", false}, {"message", "CV not found or already deleted"}}).dump(),
                    "application/json"
                );
                res.status = 404;
                std::cout << "[DELETE CV] ✗ CV " << cvId << " not found" << std::endl;
            }
            
        } catch (std::exception& e) {
            std::cerr << "[DELETE CV ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
            res.status = 500;
        }
    });


    //////////////////////////Comapny endpoints//////////////////////////////
        // ========== COMPANY ENDPOINTS ==========
    
    // --- GET ALL ACTIVE COMPANIES (For Users) ---
    svr.Get("/api/companies/active", [](const Request& req, Response& res) {
        try {
            std::vector<CompanyRecord> companies = companydb->getActiveCompanies();
            json companyArray = json::array();
            
            for (const auto& company :  companies) {
                companyArray.push_back({
                    {"companyId", company.companyId},
                    {"companyName", std::string(company.companyName)},
                    {"jobTitle", std::string(company.jobTitle)},
                    {"requiredSkills", std::string(company.requiredSkills)},
                    {"minExperience", company.minExperience},
                    {"position", std::string(company.position)},
                    {"education", std::string(company.education)},
                    {"location", std::string(company.location)},
                    {"salary", company.salary},
                    {"description", std::string(company.description)},
                    {"postedDate", company.postedDate}
                });
            }
            
            res.set_content(
                json({{"success", true}, {"companies", companyArray}}).dump(),
                "application/json"
            );
            
            std::cout << "[COMPANIES] Served " << companies.size() << " active companies" << std::endl;
            
        } catch (std::exception& e) {
            std::cerr << "[COMPANIES ERROR] " << e.what() << std::endl;
            res. set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // --- GET ALL COMPANIES (Admin Only) ---
    svr.Get("/api/companies/all", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1 || !isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                return;
            }
            
            std::vector<CompanyRecord> companies = companydb->getAllCompanies();
            json companyArray = json::array();
            
            for (const auto& company : companies) {
                // Get application count for each company
                int32_t appCount = appdb->countApplicationsByCompany(company.companyId);
                
                companyArray.push_back({
                    {"companyId", company.companyId},
                    {"companyName", std::string(company.companyName)},
                    {"jobTitle", std::string(company.jobTitle)},
                    {"requiredSkills", std::string(company. requiredSkills)},
                    {"minExperience", company.minExperience},
                    {"position", std::string(company.position)},
                    {"education", std::string(company.education)},
                    {"location", std::string(company. location)},
                    {"salary", company.salary},
                    {"description", std::string(company.description)},
                    {"isActive", company.isActive},
                    {"postedDate", company.postedDate},
                    {"applicationCount", appCount}
                });
            }
            
            res.set_content(
                json({{"success", true}, {"companies", companyArray}}).dump(),
                "application/json"
            );
            
            std:: cout << "[ADMIN COMPANIES] Served " << companies.size() << " companies" << std::endl;
            
        } catch (std::exception& e) {
            std:: cerr << "[COMPANIES ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // --- ADD COMPANY (Admin Only) ---
    svr.Post("/api/company/add", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1 || ! isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                return;
            }
            
            auto j = json::parse(req.body);
            
            int32_t companyId = companydb->addCompany(
                j["companyName"].get<std::string>(),
                j["jobTitle"].get<std::string>(),
                j["requiredSkills"].get<std::string>(),
                j["minExperience"].get<int>(),
                j["position"].get<std::string>(),
                j["education"].get<std:: string>(),
                j["location"].get<std::string>(),
                j["salary"].get<int>(),
                j["description"].get<std::string>()
            );
            
            if (companyId > 0) {
                res. set_content(
                    json({{"success", true}, {"companyId", companyId}}).dump(),
                    "application/json"
                );
                std::cout << "[ADD COMPANY] ✓ Company ID: " << companyId << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Failed to add company"}}).dump(),
                    "application/json"
                );
            }
            
        } catch (std::exception& e) {
            std::cerr << "[ADD COMPANY ERROR] " << e. what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // --- DELETE COMPANY (Admin Only) ---
    svr.Delete("/api/company/: id", [](const Request& req, Response& res) {
        try {
            auto cookie = req. get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1 || !isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                return;
            }
            
            int32_t companyId = std::stoi(req.path_params. at("id"));
            bool deleted = companydb->deleteCompany(companyId);
            
            if (deleted) {
                res.set_content(
                    json({{"success", true}, {"message", "Company deleted"}}).dump(),
                    "application/json"
                );
                std::cout << "[DELETE COMPANY] ✓ Company " << companyId << " deleted" << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Company not found"}}).dump(),
                    "application/json"
                );
                res.status = 404;
            }
            
        } catch (std::exception& e) {
            std::cerr << "[DELETE COMPANY ERROR] " << e. what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // ========== APPLICATION ENDPOINTS ==========
    
    // --- APPLY TO COMPANY ---
    svr.Post("/api/apply", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                res.status = 401;
                return;
            }
            
            auto j = json::parse(req. body);
            int32_t companyId = j["companyId"].get<int>();
            int32_t cvId = j["cvId"].get<int>();
            
            std::cout << "[APPLY] User " << userId << " applying to Company " << companyId << " with CV " << cvId << std:: endl;
            
            // Check if already applied
            if (appdb->hasUserAppliedToCompany(userId, companyId)) {
                res.set_content(
                    json({{"success", false}, {"message", "You have already applied to this company"}}).dump(),
                    "application/json"
                );
                return;
            }
            
            // Get company requirements
            CompanyRecord* company = companydb->getCompanyById(companyId);
            if (!company) {
                res.set_content(
                    json({{"success", false}, {"message", "Company not found"}}).dump(),
                    "application/json"
                );
                res.status = 404;
                return;
            }
            
            // Get CV
            CVRecord* cv = cvdb->getCVById(cvId);
            if (!cv || cv->userId != userId) {
                delete company;
                res.set_content(
                    json({{"success", false}, {"message", "CV not found or not owned by you"}}).dump(),
                    "application/json"
                );
                res.status = 404;
                return;
            }
            
            // Calculate match score
            JobRequirement jobReq;
            jobReq.requiredSkills = company->requiredSkills;
            jobReq.minExperience = company->minExperience;
            jobReq.position = company->position;
            jobReq.education = company->education;
            jobReq.location = company->location;
            
            std::vector<CVRecord> cvList = {*cv};
            std::vector<DetailedScore> scores = findBestCVs(cvList, jobReq, 1);
            
            float matchScore = scores.empty() ? 0.5f : scores[0].finalScore;
            
            // Create application
            int32_t appId = appdb->addApplication(userId, cvId, companyId, matchScore);
            
            delete company;
            delete cv;
            
            if (appId > 0) {
                res.set_content(
                    json({
                        {"success", true}, 
                        {"applicationId", appId},
                        {"matchScore", matchScore * 100}
                    }).dump(),
                    "application/json"
                );
                std::cout << "[APPLY] ✓ Application " << appId << " created (Score: " << (matchScore * 100) << "%)" << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Failed to create application"}}).dump(),
                    "application/json"
                );
            }
            
        } catch (std::exception& e) {
            std:: cerr << "[APPLY ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // --- GET USER'S APPLICATIONS ---
    svr.Get("/api/my-applications", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1) {
                res.set_content(
                    json({{"success", false}, {"message", "Not logged in"}}).dump(),
                    "application/json"
                );
                res.status = 401;
                return;
            }
            
            std::vector<ApplicationRecord> applications = appdb->getApplicationsByUser(userId);
            json appArray = json::array();
            
            for (const auto& app : applications) {
                CompanyRecord* company = companydb->getCompanyById(app. companyId);
                CVRecord* cv = cvdb->getCVById(app.cvId);
                
                if (company && cv) {
                    appArray.push_back({
                        {"applicationId", app.applicationId},
                        {"companyName", std::string(company->companyName)},
                        {"jobTitle", std::string(company->jobTitle)},
                        {"location", std::string(company->location)},
                        {"salary", company->salary},
                        {"appliedDate", app.appliedDate},
                        {"status", std::string(app.status)},
                        {"matchScore", app.matchScore * 100}
                    });
                }
                
                if (company) delete company;
                if (cv) delete cv;
            }
            
            res.set_content(
                json({{"success", true}, {"applications", appArray}}).dump(),
                "application/json"
            );
            
            std::cout << "[MY APPLICATIONS] User " << userId << " has " << applications.size() << " applications" << std::endl;
            
        } catch (std::exception& e) {
            std::cerr << "[MY APPLICATIONS ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });
    
    // --- GET APPLICATIONS FOR COMPANY (Admin Only) ---
    svr.Get("/api/company/:id/applications", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1 || !isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                return;
            }
            
            int32_t companyId = std::stoi(req.path_params.at("id"));
            
            CompanyRecord* company = companydb->getCompanyById(companyId);
            if (!company) {
                res.set_content(
                    json({{"success", false}, {"message", "Company not found"}}).dump(),
                    "application/json"
                );
                res.status = 404;
                return;
            }
            
            std::vector<ApplicationRecord> applications = appdb->getApplicationsByCompany(companyId);
            json appArray = json::array();
            
            int rank = 1;
            for (const auto& app : applications) {
                CVRecord* cv = cvdb->getCVById(app.cvId);
                UserRecord* user = userdb->getUserById(app.userId);
                
                if (cv && user) {
                    appArray.push_back({
                        {"rank", rank++},
                        {"applicationId", app.applicationId},
                        {"userId", app.userId},
                        {"username", std::string(user->username)},
                        {"cvId", app.cvId},
                        {"name", std::string(cv->name)},
                        {"email", std::string(cv->email)},
                        {"skills", std::string(cv->skills)},
                        {"experience", cv->experience},
                        {"lastPosition", std::string(cv->lastPosition)},
                        {"education", std::string(cv->education)},
                        {"location", std::string(cv->location)},
                        {"matchScore", app.matchScore * 100},
                        {"status", std::string(app.status)},
                        {"appliedDate", app.appliedDate}
                    });
                }
                
                if (cv) delete cv;
                if (user) delete user;
            }
            
            res.set_content(
                json({
                    {"success", true}, 
                    {"company", {
                        {"companyId", company->companyId},
                        {"companyName", std::string(company->companyName)},
                        {"jobTitle", std::string(company->jobTitle)},
                        {"requiredSkills", std::string(company->requiredSkills)},
                        {"minExperience", company->minExperience}
                    }},
                    {"applications", appArray}
                }).dump(),
                "application/json"
            );
            
            delete company;
            
            std::cout << "[COMPANY APPLICATIONS] Company " << companyId << " has " << applications.size() << " applications" << std:: endl;
            
        } catch (std::exception& e) {
            std::cerr << "[COMPANY APPLICATIONS ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Server error"}}).dump(),
                "application/json"
            );
        }
    });



        // --- UPDATE APPLICATION STATUS (Admin Only) ---
    svr.Put("/api/application/:id/status", [](const Request& req, Response& res) {
        try {
            auto cookie = req.get_header_value("Cookie");
            int32_t userId = sessionMgr->getUserId(cookie);
            
            if (userId == -1 || ! isAdmin(userId)) {
                res.set_content(
                    json({{"success", false}, {"message", "Admin access required"}}).dump(),
                    "application/json"
                );
                res.status = 403;
                return;
            }
            
            int32_t applicationId = std::stoi(req.path_params. at("id"));
            auto j = json::parse(req.body);
            std::string newStatus = j["status"].get<std::string>();
            
            bool updated = appdb->updateStatus(applicationId, newStatus);
            
            if (updated) {
                res.set_content(
                    json({{"success", true}, {"message", "Status updated"}}).dump(),
                    "application/json"
                );
                std::cout << "[UPDATE STATUS] Application " << applicationId << " → " << newStatus << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Application not found"}}).dump(),
                    "application/json"
                );
                res.status = 404;
            }
            
        } catch (std::exception& e) {
            std::cerr << "[UPDATE STATUS ERROR] " << e.what() << std::endl;
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
    delete companydb;   // NEW
    delete appdb;       // NEW
    delete sessionMgr;

    return 0;
}