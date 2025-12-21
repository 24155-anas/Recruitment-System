#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <unordered_map>
#include <string>
#include <random>
#include <cstdint>
#include <iostream>

class SessionManager {
private:
    std::unordered_map<std::string, int32_t> sessions;  // sessionID -> userID

    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);

        std::string sessionId(32, '0');
        for (auto& c : sessionId) {
            c = "0123456789abcdef"[dis(gen)];
        }
        return sessionId;
    }

    std::string extractSessionId(const std::string& cookieHeader) {
        // Parse Cookie header: "SESSIONID=abc123def456..."
        size_t pos = cookieHeader.find("SESSIONID=");
        if (pos != std::string::npos) {
            return cookieHeader.substr(pos + 10, 32);
        }
        return "";
    }

public:
    SessionManager() {}

    std::string createSession(int32_t userId) {
        std::string sessionId = generateSessionId();
        sessions[sessionId] = userId;
        std::cout << "Session created: " << sessionId << " -> User " << userId << std::endl;
        return sessionId;
    }

    int32_t getUserId(const std::string& cookieHeader) {
        std::string sessionId = extractSessionId(cookieHeader);
        if (sessionId.empty()) {
            return -1;
        }

        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            return it->second;
        }
        return -1;
    }

    bool deleteSession(const std::string& cookieHeader) {
        std::string sessionId = extractSessionId(cookieHeader);
        if (sessionId.empty()) {
            return false;
        }

        auto it = sessions.find(sessionId);
        if (it != sessions.end()) {
            sessions.erase(it);
            std::cout << "Session deleted: " << sessionId << std::endl;
            return true;
        }
        return false;
    }

    void printAllSessions() {
        std::cout << "\n=== Active Sessions ===" << std::endl;
        for (const auto& session : sessions) {
            std::cout << session.first << " -> User " << session.second << std::endl;
        }
        std::cout << "Total sessions: " << sessions.size() << std::endl;
    }
};

#endif