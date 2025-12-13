#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstdlib>
#include <set>
#include "../database/DataRecords.hpp"

// ============== PDF TEXT EXTRACTION ==============

// Extract text from PDF using pdftotext command
std::string extractTextFromPDF(const std::string& pdfPath) {
    std::string outputPath = pdfPath + ".txt";
    
    // Use pdftotext command (comes with poppler-utils)
    std::string command = "pdftotext -layout \"" + pdfPath + "\" \"" + outputPath + "\"";
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "[PDF] Failed to extract text from PDF" << std::endl;
        return "";
    }
    
    // Read extracted text
    std::ifstream file(outputPath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();
    
    // Clean up temp file
    std::remove(outputPath.c_str());
    
    return text;
}

// ============== TEXT PARSING FUNCTIONS ==============

// Convert string to lowercase
std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std:: string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Extract section text between two keywords
std::string extractSection(const std::string& text, 
                           const std::vector<std::string>& startKeywords,
                           const std::vector<std::string>& endKeywords) {
    std::string lowerText = toLower(text);
    
    size_t startPos = std::string::npos;
    
    // Find start position
    for (const auto& keyword : startKeywords) {
        startPos = lowerText.find(toLower(keyword));
        if (startPos != std::string::npos) {
            startPos += keyword.length();
            break;
        }
    }
    
    if (startPos == std::string:: npos) return "";
    
    // Find end position
    size_t endPos = text.length();
    for (const auto& keyword : endKeywords) {
        size_t pos = lowerText. find(toLower(keyword), startPos);
        if (pos != std::string::npos && pos < endPos) {
            endPos = pos;
        }
    }
    
    std::string section = text.substr(startPos, endPos - startPos);
    return trim(section);
}

// ============== CV FIELD EXTRACTION ==============

struct ParsedCV {
    std::string name;
    std::string email;
    std::string skills;
    int experience = 0;
    std::string lastPosition;
    std::string education;
    std::string location;
    bool success = false;
    std::string errorMessage;
};

// Extract name (usually first line or after "Name:" keyword)
std::string extractName(const std::string& text) {
    std::regex nameRegex(R"((? :name\s*: ?\s*)?([A-Z][a-z]+(? :\s+[A-Z][a-z]+)+))", 
                         std::regex::icase);
    std::smatch match;
    
    if (std::regex_search(text, match, nameRegex)) {
        return trim(match[1].str());
    }
    
    // Fallback:  first line
    size_t newline = text.find('\n');
    if (newline != std:: string::npos) {
        return trim(text.substr(0, newline));
    }
    
    return "";
}

// Extract email
std::string extractEmail(const std::string& text) {
    std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    std::smatch match;
    
    if (std::regex_search(text, match, emailRegex)) {
        return match[0].str();
    }
    
    return "";
}

// Extract skills
std::string extractSkills(const std::string& text) {
    std::vector<std::string> skillKeywords = {
        "skills:", "technical skills:", "skills", "technologies:", 
        "expertise:", "proficient in:"
    };
    
    std::vector<std::string> endKeywords = {
        "experience:", "work experience:", "education:", 
        "employment:", "projects:"
    };
    
    std::string skillSection = extractSection(text, skillKeywords, endKeywords);
    
    if (skillSection.empty()) {
        // Try to find common programming languages
        std::regex skillRegex(R"(\b(C\+\+|Python|Java|JavaScript|React|Node\.js|SQL|Docker|AWS|TensorFlow|Machine Learning|ML|AI)\b)", 
                              std::regex::icase);
        
        std::set<std::string> foundSkills;
        std::sregex_iterator iter(text.begin(), text.end(), skillRegex);
        std::sregex_iterator end;
        
        while (iter != end) {
            foundSkills.insert((*iter)[0]. str());
            ++iter;
        }
        
        std::string result;
        for (const auto& skill : foundSkills) {
            if (!result.empty()) result += ",";
            result += skill;
        }
        return result;
    }
    
    // Clean up skill section
    skillSection = std::regex_replace(skillSection, std::regex(R"(\n+)"), ",");
    skillSection = std::regex_replace(skillSection, std::regex(R"(\s*[•▪▫◦○●]\s*)"), ",");
    skillSection = std::regex_replace(skillSection, std::regex(R"(\s*,\s*)"), ",");
    
    return trim(skillSection);
}

// Extract years of experience
int extractExperience(const std::string& text) {
    // Pattern 1: "5 years of experience"
    std::regex expRegex1(R"((\d+)\s*(? :\+)?\s*years?\s+(? : of\s+)?experience)", 
                         std::regex::icase);
    std::smatch match;
    
    if (std:: regex_search(text, match, expRegex1)) {
        return std::stoi(match[1].str());
    }
    
    // Pattern 2: "Experience:  5 years"
    std::regex expRegex2(R"(experience\s*: ?\s*(\d+)\s*years? )", 
                         std::regex::icase);
    
    if (std::regex_search(text, match, expRegex2)) {
        return std:: stoi(match[1].str());
    }
    
    // Pattern 3: Count work experience entries (2020-2023 = 3 years)
    std::regex dateRangeRegex(R"((\d{4})\s*[-–]\s*(? : (\d{4})|present|current))", 
                              std::regex::icase);
    
    std::sregex_iterator iter(text. begin(), text.end(), dateRangeRegex);
    std::sregex_iterator end;
    
    int totalYears = 0;
    while (iter != end) {
        int startYear = std::stoi((*iter)[1].str());
        int endYear = (*iter)[2].matched ? std::stoi((*iter)[2].str()) : 2025;
        totalYears += (endYear - startYear);
        ++iter;
    }
    
    return totalYears;
}

// Extract last position/job title
std::string extractLastPosition(const std::string& text) {
    std::vector<std::string> posKeywords = {
        "current position:", "position:", "job title:", 
        "designation:", "role:"
    };
    
    std::vector<std::string> endKeywords = {
        "company:", "responsibilities:", "skills:", "education:"
    };
    
    std::string position = extractSection(text, posKeywords, endKeywords);
    
    if (position.empty()) {
        // Try to find common job titles
        std::regex titleRegex(R"(\b(Software Engineer|Developer|Engineer|Analyst|Manager|Lead|Senior|Junior|Architect|Scientist|Designer|Consultant)\b)", 
                              std:: regex::icase);
        std::smatch match;
        
        if (std::regex_search(text, match, titleRegex)) {
            return match[0].str();
        }
    }
    
    // Take first line
    size_t newline = position.find('\n');
    if (newline != std::string:: npos) {
        position = position.substr(0, newline);
    }
    
    return trim(position);
}

// Extract education
std::string extractEducation(const std::string& text) {
    std::vector<std::string> eduKeywords = {
        "education:", "academic background:", "qualifications:", 
        "degree:", "university:"
    };
    
    std::vector<std::string> endKeywords = {
        "experience:", "skills:", "certifications:", "projects:"
    };
    
    std::string eduSection = extractSection(text, eduKeywords, endKeywords);
    
    if (eduSection. empty()) {
        // Try to find common degrees
        std::regex degreeRegex(R"(\b(PhD|Ph\.D|Masters? |MS|M\.S|Bachelor'? s? |BS|B\.S|B\.Tech|M\.Tech|MBA|Diploma)\s+(?:of|in|of Science in|of Arts in)?\s*([A-Za-z\s]+))", 
                               std:: regex::icase);
        std::smatch match;
        
        if (std::regex_search(text, match, degreeRegex)) {
            return match[0].str();
        }
    }
    
    // Take first line
    size_t newline = eduSection.find('\n');
    if (newline != std::string::npos) {
        eduSection = eduSection.substr(0, newline);
    }
    
    return trim(eduSection);
}

// Extract location
std::string extractLocation(const std::string& text) {
    // Common Pakistani cities
    std::vector<std:: string> cities = {
        "Karachi", "Lahore", "Islamabad", "Rawalpindi", "Faisalabad",
        "Multan", "Peshawar", "Quetta", "Sialkot", "Gujranwala"
    };
    
    for (const auto& city : cities) {
        if (toLower(text).find(toLower(city)) != std::string::npos) {
            return city;
        }
    }
    
    // Try regex pattern
    std::regex locRegex(R"((? :location|address|city)\s*:?\s*([A-Z][a-z]+(?: ,\s*[A-Z][a-z]+)*))", 
                        std:: regex::icase);
    std::smatch match;
    
    if (std::regex_search(text, match, locRegex)) {
        return trim(match[1].str());
    }
    
    return "Unknown";
}

// ============== MAIN PARSING FUNCTION ==============

ParsedCV parseCV(const std:: string& pdfPath) {
    ParsedCV result;
    
    std::cout << "[PDF] Extracting text from: " << pdfPath << std::endl;
    
    // Step 1: Extract text from PDF
    std::string text = extractTextFromPDF(pdfPath);
    
    if (text.empty()) {
        result.errorMessage = "Failed to extract text from PDF";
        return result;
    }
    
    std::cout << "[PDF] Extracted " << text.length() << " characters" << std::endl;
    
    // Step 2: Parse fields
    result.name = extractName(text);
    result.email = extractEmail(text);
    result.skills = extractSkills(text);
    result.experience = extractExperience(text);
    result.lastPosition = extractLastPosition(text);
    result.education = extractEducation(text);
    result.location = extractLocation(text);
    
    // Step 3: Validation
    if (result.name.empty()) {
        result.errorMessage = "Could not extract name from CV";
        return result;
    }
    
    if (result.email. empty()) {
        result.errorMessage = "Could not extract email from CV";
        return result;
    }
    
    if (result.skills.empty()) {
        result.errorMessage = "Could not extract skills from CV";
        return result;
    }
    
    result.success = true;
    return result;
}