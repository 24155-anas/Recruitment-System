#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <fstream>


// Include your database files
#include "database/CandidateDB.hpp"

using namespace std;

// Server configuration
const int PORT = 8080;
const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 4096;

// Global database instance
mutex db_mutex;
CandidateDB* global_db = nullptr;

// Function to parse HTTP request
map<string, string> parseHttpRequest(const string& request) {
    map<string, string> result;
    stringstream ss(request);
    string line;
    
    // Parse first line (method, path)
    getline(ss, line);
    size_t method_end = line.find(' ');
    if (method_end != string::npos) {
        result["method"] = line.substr(0, method_end);
        size_t path_end = line.find(' ', method_end + 1);
        if (path_end != string::npos) {
            result["path"] = line.substr(method_end + 1, path_end - method_end - 1);
        }
    }
    
    // Parse headers
    while (getline(ss, line) && line != "\r") {
        size_t colon_pos = line.find(':');
        if (colon_pos != string::npos) {
            string key = line.substr(0, colon_pos);
            string value = line.substr(colon_pos + 2); // Skip ": "
            // Remove trailing \r
            if (!value.empty() && value.back() == '\r') {
                value.pop_back();
            }
            result[key] = value;
        }
    }
    
    // Parse body (if any)
    string body;
    while (getline(ss, line)) {
        body += line + "\n";
    }
    if (!body.empty()) {
        result["body"] = body;
    }
    
    return result;
}

// Simple JSON parser (for basic key-value pairs)
map<string, string> parseSimpleJson(const string& json_str) {
    map<string, string> result;
    string str = json_str;
    
    // Remove whitespace and braces
    size_t start = str.find('{');
    size_t end = str.find('}');
    if (start != string::npos && end != string::npos) {
        str = str.substr(start + 1, end - start - 1);
    }
    
    // Split by commas
    size_t pos = 0;
    while (pos < str.length()) {
        // Find key
        size_t quote1 = str.find('"', pos);
        if (quote1 == string::npos) break;
        size_t quote2 = str.find('"', quote1 + 1);
        if (quote2 == string::npos) break;
        string key = str.substr(quote1 + 1, quote2 - quote1 - 1);
        
        // Find colon
        size_t colon = str.find(':', quote2);
        if (colon == string::npos) break;
        
        // Find value
        size_t value_start = str.find_first_not_of(" \t\r\n", colon + 1);
        if (value_start == string::npos) break;
        
        string value;
        if (str[value_start] == '"') {
            // String value
            size_t quote3 = str.find('"', value_start + 1);
            if (quote3 == string::npos) break;
            value = str.substr(value_start + 1, quote3 - value_start - 1);
            pos = quote3 + 1;
        } else {
            // Number or boolean value
            size_t comma = str.find(',', value_start);
            size_t brace = str.find('}', value_start);
            size_t end_pos = min(comma, brace);
            if (end_pos == string::npos) end_pos = str.length();
            
            value = str.substr(value_start, end_pos - value_start);
            pos = end_pos;
        }
        
        result[key] = value;
        
        // Move past comma
        if (pos < str.length() && str[pos] == ',') {
            pos++;
        }
    }
    
    return result;
}

// Create JSON response
string createJsonResponse(bool success, const string& message, const map<string, string>& data = {}) {
    stringstream json;
    json << "{\"success\":" << (success ? "true" : "false") 
         << ",\"message\":\"" << message << "\"";
    
    for (const auto& pair : data) {
        json << ",\"" << pair.first << "\":\"" << pair.second << "\"";
    }
    
    json << "}";
    return json.str();
}

// Create HTTP response
string createHttpResponse(const string& content, const string& contentType = "application/json") {
    stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: " << contentType << "\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Length: " << content.length() << "\r\n"
             << "Connection: close\r\n"
             << "\r\n"
             << content;
    
    return response.str();
}

// Handle OPTIONS request (CORS preflight)
string handleOptionsRequest() {
    stringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Access-Control-Allow-Origin: *\r\n"
             << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
             << "Access-Control-Allow-Headers: Content-Type\r\n"
             << "Content-Length: 0\r\n"
             << "Connection: close\r\n"
             << "\r\n";
    
    return response.str();
}

// Handle login request
string handleLogin(const map<string, string>& json_data) {
    string email = json_data.count("email") ? json_data.at("email") : "";
    string password = json_data.count("password") ? json_data.at("password") : "";
    
    // Simple authentication (in real app, use proper auth)
    if (email.empty() || password.empty()) {
        return createJsonResponse(false, "Email and password required");
    }
    
    // Demo authentication - accept any non-empty password
    if (password.length() < 1) {
        return createJsonResponse(false, "Invalid credentials");
    }
    
    map<string, string> user_data;
    user_data["email"] = email;
    user_data["name"] = "User"; // In real app, get from database
    
    return createJsonResponse(true, "Login successful", user_data);
}

// Handle signup request
string handleSignup(const map<string, string>& json_data) {
    string email = json_data.count("email") ? json_data.at("email") : "";
    string password = json_data.count("password") ? json_data.at("password") : "";
    string confirmPassword = json_data.count("confirmPassword") ? json_data.at("confirmPassword") : "";
    
    // Validation
    if (email.empty() || password.empty()) {
        return createJsonResponse(false, "Email and password required");
    }
    
    if (password.length() < 6) {
        return createJsonResponse(false, "Password must be at least 6 characters");
    }
    
    if (password != confirmPassword) {
        return createJsonResponse(false, "Passwords do not match");
    }
    
    // Check if user already exists (in real app)
    // For now, always succeed
    
    map<string, string> user_data;
    user_data["email"] = email;
    user_data["id"] = "user_" + to_string(time(nullptr));
    
    return createJsonResponse(true, "Account created successfully", user_data);
}

// Handle CV data save request
string handleSaveCV(const map<string, string>& json_data) {
    lock_guard<mutex> lock(db_mutex);
    
    if (!global_db) {
        return createJsonResponse(false, "Database not initialized");
    }
    
    // Extract CV data
    CandidateRecord candidate;
    
    candidate.id = 0; // Will be assigned by database
    strncpy(candidate.name, json_data.count("name") ? json_data.at("name").c_str() : "", sizeof(candidate.name)-1);
    strncpy(candidate.email, json_data.count("email") ? json_data.at("email").c_str() : "", sizeof(candidate.email)-1);
    strncpy(candidate.skills, json_data.count("skills") ? json_data.at("skills").c_str() : "", sizeof(candidate.skills)-1);
    
    // Parse experience
    try {
        candidate.experience = json_data.count("experience") ? stoi(json_data.at("experience")) : 0;
    } catch (...) {
        candidate.experience = 0;
    }
    
    strncpy(candidate.lastPosition, json_data.count("lastPosition") ? json_data.at("lastPosition").c_str() : "", 
            sizeof(candidate.lastPosition)-1);
    
    // Optional fields
    if (json_data.count("education")) {
        // Could store in skills or separate field
        string skills = candidate.skills;
        if (!skills.empty() && skills.back() != ',') skills += ", ";
        skills += "Education: " + json_data.at("education");
        strncpy(candidate.skills, skills.c_str(), sizeof(candidate.skills)-1);
    }
    
    // Validate required fields
    if (strlen(candidate.name) == 0 || strlen(candidate.email) == 0 || 
        strlen(candidate.skills) == 0 || strlen(candidate.lastPosition) == 0) {
        return createJsonResponse(false, "Missing required fields");
    }
    
    // Add to database
    int32_t id = global_db->addCandidate(candidate);
    
    map<string, string> response_data;
    response_data["id"] = to_string(id);
    response_data["name"] = candidate.name;
    
    // Get current timestamp
    time_t now = time(nullptr);
    char timestamp[100];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    response_data["timestamp"] = timestamp;
    
    return createJsonResponse(true, "CV data stored in B-tree database", response_data);
}

// Handle search request
string handleSearch(const map<string, string>& json_data) {
    lock_guard<mutex> lock(db_mutex);
    
    if (!global_db) {
        return createJsonResponse(false, "Database not initialized");
    }
    
    string search_type = json_data.count("type") ? json_data.at("type") : "";
    
    if (search_type == "id") {
        int32_t id = json_data.count("id") ? stoi(json_data.at("id")) : 0;
        CandidateRecord* record = global_db->searchById(id);
        
        if (record) {
            map<string, string> record_data;
            record_data["id"] = to_string(record->id);
            record_data["name"] = record->name;
            record_data["email"] = record->email;
            record_data["skills"] = record->skills;
            record_data["experience"] = to_string(record->experience);
            record_data["lastPosition"] = record->lastPosition;
            
            delete record;
            return createJsonResponse(true, "Record found", record_data);
        } else {
            return createJsonResponse(false, "Record not found");
        }
    }
    
    return createJsonResponse(false, "Invalid search type");
}

// Handle client connection
void handleClient(int client_socket) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    
    // Read request
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }
    
    // Parse HTTP request
    string request_str(buffer);
    auto request = parseHttpRequest(request_str);
    
    string method = request["method"];
    string path = request["path"];
    
    cout << "[" << time(nullptr) << "] " << method << " " << path << endl;
    
    string response;
    
    // Handle CORS preflight
    if (method == "OPTIONS") {
        response = handleOptionsRequest();
    }
    // Handle API endpoints
    else if (method == "POST") {
        if (path == "/api/login") {
            auto json_data = parseSimpleJson(request["body"]);
            string json_response = handleLogin(json_data);
            response = createHttpResponse(json_response);
        }
        else if (path == "/api/signup") {
            auto json_data = parseSimpleJson(request["body"]);
            string json_response = handleSignup(json_data);
            response = createHttpResponse(json_response);
        }
        else if (path == "/api/save-cv") {
            auto json_data = parseSimpleJson(request["body"]);
            string json_response = handleSaveCV(json_data);
            response = createHttpResponse(json_response);
        }
        else if (path == "/api/search") {
            auto json_data = parseSimpleJson(request["body"]);
            string json_response = handleSearch(json_data);
            response = createHttpResponse(json_response);
        }
        else {
            response = createHttpResponse(createJsonResponse(false, "Endpoint not found"));
        }
    }
    // Handle GET requests
    else if (method == "GET") {
        if (path == "/api/status") {
            map<string, string> status_data;
            status_data["status"] = "online";
            status_data["database"] = global_db ? "connected" : "disconnected";
            status_data["timestamp"] = to_string(time(nullptr));
            
            response = createHttpResponse(createJsonResponse(true, "Server is running", status_data));
        }
        else if (path == "/" || path == "/index.html") {
            // Serve the frontend HTML file
            ifstream file("index.html");
            if (file) {
                stringstream content;
                content << file.rdbuf();
                response = createHttpResponse(content.str(), "text/html");
            } else {
                response = createHttpResponse(createJsonResponse(false, "Frontend not found"));
            }
        }
        else {
            response = createHttpResponse(createJsonResponse(false, "Invalid request"));
        }
    }
    else {
        response = createHttpResponse(createJsonResponse(false, "Method not allowed"));
    }
    
    // Send response
    send(client_socket, response.c_str(), response.length(), 0);
    
    // Close connection
    close(client_socket);
}

int main() {
    cout << "=== Recruitment System Backend Server ===" << endl;
    cout << "Starting server on port " << PORT << "..." << endl;
    
    // Initialize database
    try {
        global_db = new CandidateDB("recruitment_index.dat", "recruitment_data.dat");
        cout << "Database initialized successfully" << endl;
        cout << "Index file: recruitment_index.dat" << endl;
        cout << "Data file: recruitment_data.dat" << endl;
    } catch (const exception& e) {
        cerr << "Failed to initialize database: " << e.what() << endl;
        return 1;
    }
    
    // Create socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        cerr << "Failed to create socket" << endl;
        delete global_db;
        return 1;
    }
    
    // Set socket options (reuse address)
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        cerr << "Failed to set socket options" << endl;
        close(server_socket);
        delete global_db;
        return 1;
    }
    
    // Bind socket
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Failed to bind socket" << endl;
        close(server_socket);
        delete global_db;
        return 1;
    }
    
    // Listen for connections
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        cerr << "Failed to listen on socket" << endl;
        close(server_socket);
        delete global_db;
        return 1;
    }
    
    cout << "Server is listening on http://localhost:" << PORT << endl;
    cout << "Press Ctrl+C to stop the server" << endl;
    
    // Main server loop
    while (true) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Accept connection
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket < 0) {
            cerr << "Failed to accept connection" << endl;
            continue;
        }
        
        // Get client IP
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        
        cout << "[" << time(nullptr) << "] Connection from " << client_ip << endl;
        
        // Handle client in a separate thread
        thread client_thread(handleClient, client_socket);
        client_thread.detach(); // Detach thread to run independently
    }
    
    // Cleanup (not normally reached)
    close(server_socket);
    delete global_db;
    
    return 0;
}