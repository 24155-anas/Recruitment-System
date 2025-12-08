

//add mutex

#include "httplib.h"
#include "json.hpp"
#include "../database/UserDB.hpp"
#include "../database/CVDB.hpp"
#include "SessionManager.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;
using namespace httplib;
using namespace std;

// Global database instances
UserDatabase* userdb = nullptr;
CVDatabase* cvdb = nullptr;
SessionManager* sessionMgr = nullptr;

// Helper function to read files
std::string read_file(const std::string& filename) {
    std::string filepath = "../frontend/" + filename;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR: File not found: " << filepath << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    std::cout << "✓ Served: " << filepath << std::endl;
    return content;
}

int main() {
    std::cout << "\n=== Recruitment System Server ===" << std::endl;
    std::cout << "Current directory: " << std::filesystem::current_path() << std::endl;
    
    // Initialize databases
    std::cout << "Initializing databases..." << std::endl;
    userdb = new UserDatabase("../database/user.idx", "../database/user.dat");
    cvdb = new CVDatabase("../database/cv.idx", "../database/cv.dat");
    sessionMgr = new SessionManager();

    Server svr;

    // Set logger
    svr.set_logger([](const Request& req, const Response& res) {
        std::cout << "REQUEST: " << req.method << " " << req.path << " -> " << res.status << std::endl;
    });

    // --- SERVE ROOT ---
    svr.Get("/", [](const Request&, Response& res) {
        std::string content = read_file("index.html");
        if (!content.empty()) {
            res.set_content(content, "text/html");
        } else {
            res.set_content("<h1>500 Error: index.html not found</h1>", "text/html");
            res.status = 500;
        }
    });



    // --- SERVE index.html EXPLICITLY ---
    svr.Get("/index.html", [](const Request&, Response& res) {
        std::cout << "GET /index.html called" << std::endl;
        std::string content = read_file("index.html");
        if (!content.empty()) {
            std::cout << "Serving index. html (" << content.length() << " bytes)" << std::endl;
            res.set_content(content, "text/html");
        } else {
            std::cout << "ERROR: Could not read index.html" << std::endl;
            res.set_content("<h1>500 Error: index.html not found</h1>", "text/html");
            res.status = 500;
        }
    });

    // --- SERVE dashboard.html ---
    svr.Get("/dashboard.html", [](const Request&, Response& res) {
        std::cout << "GET /dashboard.html called" << std::endl;
        std::string content = read_file("dashboard.html");
        if (!content.empty()) {
            std::cout << "Serving dashboard.html (" << content.length() << " bytes)" << std::endl;
            res.set_content(content, "text/html");
        } else {
            std::cout << "ERROR: Could not read dashboard.html" << std::endl;
            res.set_content("<h1>500 Error: dashboard.html not found</h1>", "text/html");
            res. status = 500;
        }
    });

    // --- SERVE cv-form.html ---
    svr. Get("/cv-form.html", [](const Request&, Response& res) {
        std::cout << "GET /cv-form. html called" << std::endl;
        std::string content = read_file("cv-form.html");
        if (!content.empty()) {
            res.set_content(content, "text/html");
        } else {
            res.set_content("<h1>500 Error: cv-form. html not found</h1>", "text/html");
            res.status = 500;
        }
    });

    // --- SERVE styles.css ---
    svr. Get("/styles.css", [](const Request&, Response& res) {
        std::string content = read_file("styles.css");
        if (!content. empty()) {
            res.set_content(content, "text/css");
        } else {
            res.set_content("/* Error: styles.css not found */", "text/css");
        }
    });

    // --- SIGNUP ENDPOINT ---
    svr. Post("/signup", [](const Request& req, Response& res) {
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
                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly");
                res.set_content(json({{"success", true}}).dump(), "application/json");
                std::cout << "[SIGNUP] ✓ User registered: " << username << " (ID: " << userId << ")" << std::endl;
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "Failed to register"}}).dump(),
                    "application/json"
                );
            }
        } catch (std::exception& e) {
            std::cerr << "[SIGNUP ERROR] " << e.what() << std::endl;
            res. set_content(
                json({{"success", false}, {"message", "Invalid request"}}).dump(),
                "application/json"
            );
        }
    });

    // --- LOGIN ENDPOINT ---
    svr.Post("/login", [](const Request& req, Response& res) {
        try {
            auto j = json::parse(req.body);
            std::string username = j["username"];
            std::string password = j["password"];

            std::cout << "[LOGIN] Username: " << username << std::endl;

            int32_t userId = userdb->loginUser(username, password);
            if (userId > 0) {
                std::string sessionId = sessionMgr->createSession(userId);
                res.set_header("Set-Cookie", "SESSIONID=" + sessionId + "; Path=/; HttpOnly");
                res.set_content(json({{"success", true}}).dump(), "application/json");
                std::cout << "[LOGIN] ✓ User logged in: " << username << " (ID: " << userId << ")" << std::endl;
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
    svr.Post("/logout", [](const Request& req, Response& res) {
        auto cookie = req.get_header_value("Cookie");
        std::cout << "[LOGOUT] Cookie: " << cookie << std::endl;
        sessionMgr->deleteSession(cookie);
        res. set_content(json({{"success", true}}).dump(), "application/json");
    });

    // --- SUBMIT CV ENDPOINT ---
    svr.Post("/submit_cv", [](const Request& req, Response& res) {
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

            auto j = json::parse(req. body);
            int32_t cvId = cvdb->addCV(
                userId,
                j["name"].get<std::string>(),
                j["email"].get<std::string>(),
                j["skills"].get<std::string>(),
                std::stoi(j["experience"]. get<std::string>()),
                j["lastPosition"].get<std::string>(),
                j["education"].get<std::string>(),
                j["location"].get<std::string>()
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

    // --- GET CV ENDPOINT ---
    svr.Get("/get_cv", [](const Request& req, Response& res) {
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

            CVRecord* cvPtr = cvdb->getCVByUserId(userId);
            if (cvPtr) {
                json jcv = {
                    {"cvId", cvPtr->cvId},
                    {"userId", cvPtr->userId},
                    {"name", cvPtr->name},
                    {"email", cvPtr->email},
                    {"skills", cvPtr->skills},
                    {"experience", cvPtr->experience},
                    {"lastPosition", cvPtr->lastPosition},
                    {"education", cvPtr->education},
                    {"location", cvPtr->location}
                };
                delete cvPtr;
                res.set_content(
                    json({{"success", true}, {"cv", jcv}}).dump(),
                    "application/json"
                );
            } else {
                res.set_content(
                    json({{"success", false}, {"message", "No CV found"}}).dump(),
                    "application/json"
                );
            }
        } catch (std::exception& e) {
            std::cerr << "[GET CV ERROR] " << e.what() << std::endl;
            res.set_content(
                json({{"success", false}, {"message", "Error retrieving CV"}}).dump(),
                "application/json"
            );
        }
    });

    // Start server
    std::cout << "Server running on http://localhost:8080/" << std::endl;
    std::cout << "Press Ctrl+C to stop\n" << std::endl;

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