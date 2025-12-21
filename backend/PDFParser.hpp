// #pragma once
// #include <string>
// #include <vector>
// #include <fstream>
// #include <sstream>
// #include <regex>
// #include <algorithm>
// #include <cstdlib>
// #include <set>
// #include "../database/DataRecords.hpp"

// // ============== PDF TEXT EXTRACTION ==============

// // Extract text from PDF using pdftotext command
// std::string extractTextFromPDF(const std::string& pdfPath) {
//     std::string outputPath = pdfPath + ".txt";
    
//     // Use pdftotext command (comes with poppler-utils)
//     std::string command = "pdftotext -layout \"" + pdfPath + "\" \"" + outputPath + "\"";
//     int result = system(command.c_str());
    
//     if (result != 0) {
//         std::cerr << "[PDF] Failed to extract text from PDF" << std::endl;
//         return "";
//     }
    
//     // Read extracted text
//     std::ifstream file(outputPath);
//     if (!file.is_open()) {
//         return "";
//     }
    
//     std::stringstream buffer;
//     buffer << file.rdbuf();
//     std::string text = buffer.str();
    
//     // Clean up temp file
//     std::remove(outputPath.c_str());
    
//     return text;
// }

// // ============== TEXT PARSING FUNCTIONS ==============

// // Convert string to lowercase
// std::string toLower(const std::string& str) {
//     std::string result = str;
//     std::transform(result.begin(), result.end(), result.begin(), ::tolower);
//     return result;
// }

// // Trim whitespace
// std::string trim(const std::string& str) {
//     size_t first = str.find_first_not_of(" \t\n\r");
//     if (first == std:: string::npos) return "";
//     size_t last = str.find_last_not_of(" \t\n\r");
//     return str.substr(first, (last - first + 1));
// }

// // Extract section text between two keywords
// std::string extractSection(const std::string& text, 
//                            const std::vector<std::string>& startKeywords,
//                            const std::vector<std::string>& endKeywords) {
//     std::string lowerText = toLower(text);
    
//     size_t startPos = std::string::npos;
    
//     // Find start position
//     for (const auto& keyword : startKeywords) {
//         startPos = lowerText.find(toLower(keyword));
//         if (startPos != std::string::npos) {
//             startPos += keyword.length();
//             break;
//         }
//     }
    
//     if (startPos == std::string:: npos) return "";
    
//     // Find end position
//     size_t endPos = text.length();
//     for (const auto& keyword : endKeywords) {
//         size_t pos = lowerText. find(toLower(keyword), startPos);
//         if (pos != std::string::npos && pos < endPos) {
//             endPos = pos;
//         }
//     }
    
//     std::string section = text.substr(startPos, endPos - startPos);
//     return trim(section);
// }

// // ============== CV FIELD EXTRACTION ==============

// struct ParsedCV {
//     std::string name;
//     std::string email;
//     std::string skills;
//     int experience = 0;
//     std::string lastPosition;
//     std::string education;
//     std::string location;
//     bool success = false;
//     std::string errorMessage;
// };

// // Extract name (usually first line or after "Name:" keyword)
// std::string extractName(const std::string& text) {
//     std::regex nameRegex(R"((? :name\s*: ?\s*)?([A-Z][a-z]+(? :\s+[A-Z][a-z]+)+))", 
//                          std::regex::icase);
//     std::smatch match;
    
//     if (std::regex_search(text, match, nameRegex)) {
//         return trim(match[1].str());
//     }
    
//     // Fallback:  first line
//     size_t newline = text.find('\n');
//     if (newline != std:: string::npos) {
//         return trim(text.substr(0, newline));
//     }
    
//     return "";
// }

// // Extract email
// std::string extractEmail(const std::string& text) {
//     std::regex emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
//     std::smatch match;
    
//     if (std::regex_search(text, match, emailRegex)) {
//         return match[0].str();
//     }
    
//     return "";
// }

// // Extract skills
// std::string extractSkills(const std::string& text) {
//     std::vector<std::string> skillKeywords = {
//         "skills:", "technical skills:", "skills", "technologies:", 
//         "expertise:", "proficient in:"
//     };
    
//     std::vector<std::string> endKeywords = {
//         "experience:", "work experience:", "education:", 
//         "employment:", "projects:"
//     };
    
//     std::string skillSection = extractSection(text, skillKeywords, endKeywords);
    
//     if (skillSection.empty()) {
//         // Try to find common programming languages
//         std::regex skillRegex(R"(\b(C\+\+|Python|Java|JavaScript|React|Node\.js|SQL|Docker|AWS|TensorFlow|Machine Learning|ML|AI)\b)", 
//                               std::regex::icase);
        
//         std::set<std::string> foundSkills;
//         std::sregex_iterator iter(text.begin(), text.end(), skillRegex);
//         std::sregex_iterator end;
        
//         while (iter != end) {
//             foundSkills.insert((*iter)[0]. str());
//             ++iter;
//         }
        
//         std::string result;
//         for (const auto& skill : foundSkills) {
//             if (!result.empty()) result += ",";
//             result += skill;
//         }
//         return result;
//     }
    
//     // Clean up skill section
//     skillSection = std::regex_replace(skillSection, std::regex(R"(\n+)"), ",");
//     skillSection = std::regex_replace(skillSection, std::regex(R"(\s*[•▪▫◦○●]\s*)"), ",");
//     skillSection = std::regex_replace(skillSection, std::regex(R"(\s*,\s*)"), ",");
    
//     return trim(skillSection);
// }

// // Extract years of experience
// int extractExperience(const std::string& text) {
//     // Pattern 1: "5 years of experience"
//     std::regex expRegex1(R"((\d+)\s*(? :\+)?\s*years?\s+(? : of\s+)?experience)", 
//                          std::regex::icase);
//     std::smatch match;
    
//     if (std:: regex_search(text, match, expRegex1)) {
//         return std::stoi(match[1].str());
//     }
    
//     // Pattern 2: "Experience:  5 years"
//     std::regex expRegex2(R"(experience\s*: ?\s*(\d+)\s*years? )", 
//                          std::regex::icase);
    
//     if (std::regex_search(text, match, expRegex2)) {
//         return std:: stoi(match[1].str());
//     }
    
//     // Pattern 3: Count work experience entries (2020-2023 = 3 years)
//     std::regex dateRangeRegex(R"((\d{4})\s*[-–]\s*(? : (\d{4})|present|current))", 
//                               std::regex::icase);
    
//     std::sregex_iterator iter(text. begin(), text.end(), dateRangeRegex);
//     std::sregex_iterator end;
    
//     int totalYears = 0;
//     while (iter != end) {
//         int startYear = std::stoi((*iter)[1].str());
//         int endYear = (*iter)[2].matched ? std::stoi((*iter)[2].str()) : 2025;
//         totalYears += (endYear - startYear);
//         ++iter;
//     }
    
//     return totalYears;
// }

// // Extract last position/job title
// std::string extractLastPosition(const std::string& text) {
//     std::vector<std::string> posKeywords = {
//         "current position:", "position:", "job title:", 
//         "designation:", "role:"
//     };
    
//     std::vector<std::string> endKeywords = {
//         "company:", "responsibilities:", "skills:", "education:"
//     };
    
//     std::string position = extractSection(text, posKeywords, endKeywords);
    
//     if (position.empty()) {
//         // Try to find common job titles
//         std::regex titleRegex(R"(\b(Software Engineer|Developer|Engineer|Analyst|Manager|Lead|Senior|Junior|Architect|Scientist|Designer|Consultant)\b)", 
//                               std:: regex::icase);
//         std::smatch match;
        
//         if (std::regex_search(text, match, titleRegex)) {
//             return match[0].str();
//         }
//     }
    
//     // Take first line
//     size_t newline = position.find('\n');
//     if (newline != std::string:: npos) {
//         position = position.substr(0, newline);
//     }
    
//     return trim(position);
// }

// // Extract education
// std::string extractEducation(const std::string& text) {
//     std::vector<std::string> eduKeywords = {
//         "education:", "academic background:", "qualifications:", 
//         "degree:", "university:"
//     };
    
//     std::vector<std::string> endKeywords = {
//         "experience:", "skills:", "certifications:", "projects:"
//     };
    
//     std::string eduSection = extractSection(text, eduKeywords, endKeywords);
    
//     if (eduSection. empty()) {
//         // Try to find common degrees
//         std::regex degreeRegex(R"(\b(PhD|Ph\.D|Masters? |MS|M\.S|Bachelor'? s? |BS|B\.S|B\.Tech|M\.Tech|MBA|Diploma)\s+(?:of|in|of Science in|of Arts in)?\s*([A-Za-z\s]+))", 
//                                std:: regex::icase);
//         std::smatch match;
        
//         if (std::regex_search(text, match, degreeRegex)) {
//             return match[0].str();
//         }
//     }
    
//     // Take first line
//     size_t newline = eduSection.find('\n');
//     if (newline != std::string::npos) {
//         eduSection = eduSection.substr(0, newline);
//     }
    
//     return trim(eduSection);
// }

// // Extract location
// std::string extractLocation(const std::string& text) {
//     // Common Pakistani cities
//     std::vector<std:: string> cities = {
//         "Karachi", "Lahore", "Islamabad", "Rawalpindi", "Faisalabad",
//         "Multan", "Peshawar", "Quetta", "Sialkot", "Gujranwala"
//     };
    
//     for (const auto& city : cities) {
//         if (toLower(text).find(toLower(city)) != std::string::npos) {
//             return city;
//         }
//     }
    
//     // Try regex pattern
//     std::regex locRegex(R"((? :location|address|city)\s*:?\s*([A-Z][a-z]+(?: ,\s*[A-Z][a-z]+)*))", 
//                         std:: regex::icase);
//     std::smatch match;
    
//     if (std::regex_search(text, match, locRegex)) {
//         return trim(match[1].str());
//     }
    
//     return "Unknown";
// }

// // ============== MAIN PARSING FUNCTION ==============

// ParsedCV parseCV(const std:: string& pdfPath) {
//     ParsedCV result;
    
//     std::cout << "[PDF] Extracting text from: " << pdfPath << std::endl;
    
//     // Step 1: Extract text from PDF
//     std::string text = extractTextFromPDF(pdfPath);
    
//     if (text.empty()) {
//         result.errorMessage = "Failed to extract text from PDF";
//         return result;
//     }
    
//     std::cout << "[PDF] Extracted " << text.length() << " characters" << std::endl;
    
//     // Step 2: Parse fields
//     result.name = extractName(text);
//     result.email = extractEmail(text);
//     result.skills = extractSkills(text);
//     result.experience = extractExperience(text);
//     result.lastPosition = extractLastPosition(text);
//     result.education = extractEducation(text);
//     result.location = extractLocation(text);
    
//     // Step 3: Validation
//     if (result.name.empty()) {
//         result.errorMessage = "Could not extract name from CV";
//         return result;
//     }
    
//     if (result.email. empty()) {
//         result.errorMessage = "Could not extract email from CV";
//         return result;
//     }
    
//     if (result.skills.empty()) {
//         result.errorMessage = "Could not extract skills from CV";
//         return result;
//     }
    
//     result.success = true;
//     return result;
// }

#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <algorithm>
#include <cstdlib>
#include <set>
#include <iostream>
#include "../database/DataRecords.hpp"

// ============== PDF TEXT EXTRACTION ==============

// Extract text from PDF using pdftotext command
std::string extractTextFromPDF(const std::string& pdfPath) {
    std::string outputPath = pdfPath + ".txt";
    
    // Use pdftotext command (comes with poppler-utils)
    std::string command = "pdftotext -layout \"" + pdfPath + "\" \"" + outputPath + "\"";
    int result = system(command.c_str());
    
    if (result != 0) {
        std::cerr << "[PDF] pdftotext failed, falling back to simple extraction" << std::endl;
        
        // Fallback:  Simple binary extraction
        std::ifstream file(pdfPath, std::ios::binary);
        if (!file) return "";
        
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        
        // Extract text between parentheses (simple PDF text)
        std::string text;
        bool inText = false;
        
        for (size_t i = 0; i < content.length(); i++) {
            if (content[i] == '(' && (i == 0 || content[i-1] != '\\')) {
                inText = true;
            } else if (content[i] == ')' && inText) {
                text += " ";
                inText = false;
            } else if (inText && content[i] >= 32 && content[i] <= 126) {
                text += content[i];
            }
        }
        
        return text;
    }
    
    // Read extracted text
    std:: ifstream file(outputPath);
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
    size_t last = str. find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// Extract section text between two keywords
std::string extractSection(const std::string& text, 
                           const std::vector<std::string>& startKeywords,
                           const std:: vector<std::string>& endKeywords) {
    std::string lowerText = toLower(text);
    
    size_t startPos = std::string:: npos;
    
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
    
    std:: string section = text.substr(startPos, endPos - startPos);
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
std::string extractName(const std:: string& text) {
    // Simple approach: first non-empty line
    std::istringstream stream(text);
    std::string line;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        
        // Skip empty lines and common headers
        if (line.empty() || 
            toLower(line).find("curriculum vitae") != std::string::npos ||
            toLower(line).find("resume") != std::string::npos) {
            continue;
        }
        
        // Skip lines with ":" (likely labels)
        if (line.find(':') != std::string::npos) {
            continue;
        }
        
        // Check if it looks like a name (2-4 words, starts with capital)
        if (line.length() > 3 && line. length() < 50 && 
            std::isupper(line[0])) {
            return line;
        }
    }
    
    return "";
}

// Extract email
std::string extractEmail(const std::string& text) {
    // Simple email regex without zero-width assertions
    std::regex emailRegex(R"([a-zA-Z0-9._+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]+)");
    std::smatch match;
    
    if (std::regex_search(text, match, emailRegex)) {
        return match[0].str();
    }
    
    return "";
}
std::string extractSkills(const std::string& text) {
    std::string lowerText = toLower(text);
    
    // ========== STEP 1: Find "SKILLS" section ==========
    size_t skillsPos = lowerText.find("skills");
    if (skillsPos == std::string::npos) {
        std::cout << "[PARSER] No 'SKILLS' section found" << std::endl;
        return "Not specified";
    }
    
    // ========== STEP 2: Skip the header line ==========
    // Find the newline AFTER "SKILLS"
    size_t headerEnd = text.find('\n', skillsPos);
    if (headerEnd == std::string::npos) {
        return "Not specified";
    }
    
    size_t contentStart = headerEnd + 1;
    
    // ========== STEP 3: Find where skills section ENDS ==========
    // Look for next major section (all caps words at start of line)
    size_t contentEnd = text.length();
    
    // Search for common section headers
    std::vector<std::string> sectionHeaders = {
        "work experience",
        "experience", 
        "education",
        "projects",
        "certifications",
        "achievements",
        "employment"
    };
    
    // Find the EARLIEST section that comes after skills
    for (const auto& header : sectionHeaders) {
        // Look for "\n" + header (section at start of new line)
        size_t searchPos = contentStart;
        
        while (searchPos < text.length()) {
            size_t pos = lowerText.find(header, searchPos);
            
            if (pos == std::string::npos) {
                break;  // Not found
            }
            
            // Check if it's at the start of a line
            if (pos == 0 || text[pos - 1] == '\n' || text[pos - 1] == '\r') {
                // Found a section header! 
                if (pos < contentEnd && pos > contentStart) {
                    contentEnd = pos;
                }
                break;
            }
            
            searchPos = pos + header.length();
        }
    }
    
    // Sanity check:  don't extract more than 1000 chars
    if (contentEnd - contentStart > 1000) {
        contentEnd = contentStart + 1000;
    }
    
    // ========== STEP 4: Extract the skills section ==========
    std:: string skillSection = text.substr(contentStart, contentEnd - contentStart);
    
    std::cout << "[PARSER] Skills section (" << skillSection.length() << " chars):" << std::endl;
    std::cout << "--- BEGIN ---" << std::endl;
    std::cout << skillSection. substr(0, 200) << "..." << std::endl;
    std::cout << "--- END ---" << std::endl;
    
    // ========== STEP 5: Parse line by line ==========
    std:: vector<std::string> skillsList;
    std::istringstream stream(skillSection);
    std::string line;
    
    while (std::getline(stream, line)) {
        line = trim(line);
        
        // Skip empty lines
        if (line. empty()) continue;
        
        // Skip if line is ALL CAPS (likely a section header)
        bool isAllCaps = true;
        int letterCount = 0;
        for (char c : line) {
            if (std::isalpha(c)) {
                letterCount++;
                if (std::islower(c)) {
                    isAllCaps = false;
                    break;
                }
            }
        }
        
        if (isAllCaps && letterCount > 0 && line.length() < 30) {
            std::cout << "[PARSER] Skipping header:  " << line << std::endl;
            continue;  // Skip section headers
        }
        
        // ========== ONLY extract lines with ":" ==========
        size_t colonPos = line.find(':');
        
        if (colonPos != std:: string::npos && colonPos < 50) {
            // Line format: "Category: value1, value2, value3"
            std::string category = line.substr(0, colonPos);
            std::string values = line.substr(colonPos + 1);
            
            category = trim(category);
            values = trim(values);
            
            // Skip if values are empty or too short
            if (values. length() < 3) continue;
            
            // Skip if it looks like a date or job info
            if (values.find("20") != std::string::npos && values.length() < 20) {
                continue;  // Likely a date
            }
            
            skillsList.push_back(values);
            
            std::cout << "[PARSER] Found skill category '" << category << "':  " << values << std::endl;
        }
    }
    
    // ========== STEP 6: Combine into single string ==========
    std::string result;
    for (const auto& skill : skillsList) {
        if (!result.empty()) result += ", ";
        result += skill;
    }
    
    // Clean up multiple commas/spaces
    result = std::regex_replace(result, std::regex(R"(\s*,\s*)"), ", ");
    result = std::regex_replace(result, std::regex(R"(,+)"), ",");
    result = trim(result);
    
    if (result.empty()) {
        std::cout << "[PARSER] No skills extracted, returning 'Not specified'" << std::endl;
        return "Not specified";
    }
    
    std::cout << "[PARSER] Final skills: " << result << std:: endl;
    
    return result;
}
// Extract years of experience
int extractExperience(const std::string& text) {
    std::string lowerText = toLower(text);
    
    // Pattern 1: "5 years" or "5 years of experience"
    std::regex expRegex1(R"((\d+)\s*\+?\s*years?)");
    std::smatch match;
    
    if (std:: regex_search(lowerText, match, expRegex1)) {
        return std:: stoi(match[1].str());
    }
    
    // Pattern 2: Count work experience date ranges (2019-2021 = 2 years)
    std::regex dateRangeRegex(R"((\d{4})\s*[-–]\s*(\d{4}|present|current))");
    
    std::sregex_iterator iter(lowerText.begin(), lowerText.end(), dateRangeRegex);
    std::sregex_iterator end;
    
    int totalYears = 0;
    while (iter != end) {
        int startYear = std::stoi((*iter)[1]. str());
        std::string endYearStr = (*iter)[2].str();
        
        int endYear;
        if (endYearStr == "present" || endYearStr == "current") {
            endYear = 2025;
        } else {
            endYear = std::stoi(endYearStr);
        }
        
        totalYears += (endYear - startYear);
        ++iter;
    }
    
    return totalYears;
}

// Extract last position/job title
std::string extractLastPosition(const std::string& text) {
    std::vector<std::string> posKeywords = {
        "current position:", "position:", "job title:", 
        "designation:", "role:", "work experience", "experience"
    };
    
    std::vector<std::string> endKeywords = {
        "company:", "responsibilities:", "skills:", "education:", "projects:"
    };
    
    std::string position = extractSection(text, posKeywords, endKeywords);
    
    if (position.empty()) {
        // Try to find common job titles
        std::vector<std::string> titles = {
            "Senior Software Engineer", "Software Engineer", "Junior Software Engineer",
            "Software Developer", "Senior Developer", "Junior Developer",
            "Full Stack Developer", "Backend Developer", "Frontend Developer",
            "Data Scientist", "Machine Learning Engineer", "DevOps Engineer",
            "Product Manager", "Project Manager", "Team Lead",
            "Engineer", "Developer", "Analyst", "Architect", "Consultant"
        };
        
        for (const auto& title : titles) {
            if (toLower(text).find(toLower(title)) != std::string::npos) {
                return title;
            }
        }
        
        return "Not specified";
    }
    
    // Take first line
    size_t newline = position.find('\n');
    if (newline != std::string::npos) {
        position = position.substr(0, newline);
    }
    
    return trim(position);
}

// Extract education
std::string extractEducation(const std::string& text) {
    std::vector<std::string> eduKeywords = {
        "education:", "education", "academic background:", "qualifications:", 
        "degree:", "university:", "college:"
    };
    
    std::vector<std::string> endKeywords = {
        "experience:", "work experience:", "skills:", "certifications:", "projects:"
    };
    
    std::string eduSection = extractSection(text, eduKeywords, endKeywords);
    
    if (eduSection.empty()) {
        // Try to find common degrees
        std::vector<std:: string> degrees = {
            "PhD", "Ph\\.D", "Doctorate",
            "Master", "Masters", "MS", "M\\.S", "M\\. Sc", "MBA", "M\\.Tech",
            "Bachelor", "Bachelors", "BS", "B\\.S", "B\\.Sc", "B\\.Tech", "B\\.E",
            "Diploma"
        };
        
        for (const auto& degree : degrees) {
            std::regex degreeRegex(degree, std::regex::icase);
            if (std::regex_search(text, degreeRegex)) {
                // Find the line containing this degree
                std::istringstream stream(text);
                std::string line;
                while (std::getline(stream, line)) {
                    if (std::regex_search(line, degreeRegex)) {
                        return trim(line);
                    }
                }
            }
        }
        
        return "Not specified";
    }
    
    // Take first meaningful line
    std::istringstream stream(eduSection);
    std::string line;
    while (std::getline(stream, line)) {
        line = trim(line);
        if (line.length() > 10) {  // Meaningful content
            return line;
        }
    }
    
    return trim(eduSection);
}

// Extract location
std::string extractLocation(const std::string& text) {
    // Common Pakistani cities
    std::vector<std:: string> cities = {
        "Karachi", "Lahore", "Islamabad", "Rawalpindi", "Faisalabad",
        "Multan", "Peshawar", "Quetta", "Sialkot", "Gujranwala",
        "Pakistan"
    };
    
    for (const auto& city : cities) {
        if (toLower(text).find(toLower(city)) != std::string::npos) {
            return city;
        }
    }
    
    // Look for "Location:" label
    std::vector<std::string> locKeywords = {"location:", "address:", "city:"};
    std::vector<std::string> endKeywords = {"email:", "phone:", "linkedin:", "\n\n"};
    
    std::string location = extractSection(text, locKeywords, endKeywords);
    if (!location.empty()) {
        // Take first line
        size_t newline = location.find('\n');
        if (newline != std::string::npos) {
            location = location.substr(0, newline);
        }
        return trim(location);
    }
    
    return "Not specified";
}

// ============== MAIN PARSING FUNCTION ==============

ParsedCV parseCV(const std:: string& pdfPath) {
    ParsedCV result;
    
    std::cout << "[PDF] Extracting text from:  " << pdfPath << std:: endl;
    
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
    
    std::cout << "[PDF] Parsed fields:" << std::endl;
    std::cout << "  Name: " << result.name << std::endl;
    std:: cout << "  Email: " << result.email << std::endl;
    std::cout << "  Skills: " << result.skills << std::endl;
    std:: cout << "  Experience: " << result.experience << " years" << std::endl;
    std:: cout << "  Position: " << result.lastPosition << std::endl;
    std:: cout << "  Education: " << result.education << std::endl;
    std::cout << "  Location: " << result.location << std::endl;
    
    // Step 3: Validation
    if (result.name. empty()) {
        result.errorMessage = "Could not extract name from CV";
        return result;
    }
    
    if (result.email. empty()) {
        result.errorMessage = "Could not extract email from CV";
        return result;
    }
    
    // Skills can be "Not specified" - don't fail on this
    if (result.skills.empty()) {
        result.skills = "Not specified";
    }
    
    result.success = true;
    std::cout << "[PDF] ✓ Parse successful" << std::endl;
    
    return result;
}