

// database/CVMatcher.hpp
#pragma once
#include <vector>
#include <string>
#include <algorithm>
#include <set>
#include <sstream>
#include <cmath>
#include "DataRecords.hpp"

// ============== JOB REQUIREMENT ==============
struct JobRequirement {
    std::string requiredSkills;      // "C++,Python,SQL"
    int minExperience;               // 5 years
    std::string position;            // "Software Engineer"
    std::string education;           // "BS CS" or "MS CS"
    std::string location;            // "Lahore" (optional)
    
    // Thresholds
    double minAcceptableScore = 0.60;  // 60% minimum to consider
};

// ============== DETAILED SCORE BREAKDOWN ==============
struct DetailedScore {
    double skillsScore = 0.0;        // 0-1
    double experienceScore = 0.0;    // 0-1
    double educationScore = 0.0;     // 0-1
    double positionScore = 0.0;      // 0-1
    double locationScore = 0.0;      // 0-1
    
    double finalScore = 0.0;         // Weighted total
    
    CVRecord cv;
    
    void print() const {
        std::cout << "\n╔══════════════ CV MATCH ANALYSIS ══════════════╗\n";
        std:: cout << "  Candidate: " << cv.name << "\n";
        std:: cout << "  ──────────────────────────────────────────────\n";
        std::cout << "  💼 Skills Score:      " << (skillsScore * 100) << "% (Weight: 40%)\n";
        std::cout << "  📅 Experience Score: " << (experienceScore * 100) << "% (Weight: 25%)\n";
        std::cout << "  🎓 Education Score:  " << (educationScore * 100) << "% (Weight: 15%)\n";
        std::cout << "  🏢 Position Score:   " << (positionScore * 100) << "% (Weight: 15%)\n";
        std::cout << "  📍 Location Score:   " << (locationScore * 100) << "% (Weight: 5%)\n";
        std::cout << "  ──────────────────────────────────────────────\n";
        std::cout << "  🎯 FINAL SCORE:      " << (finalScore * 100) << "%\n";
        std::cout << "╚═══════════════════════════════════════════════╝\n";
    }
};

// ============== UTILITY FUNCTIONS ==============

// Tokenize skills string into set
std::set<std::string> tokenizeSkills(const std:: string& skills) {
    std::set<std::string> tokens;
    std::stringstream ss(skills);
    std::string token;
    
    while (std::getline(ss, token, ',')) {
        // Trim whitespace and convert to lowercase
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        
        std::transform(token.begin(), token.end(), token.begin(), ::tolower);
        
        if (! token.empty()) {
            tokens.insert(token);
        }
    }
    return tokens;
}

// Case-insensitive string matching
bool containsIgnoreCase(const std:: string& haystack, const std::string& needle) {
    std::string h = haystack, n = needle;
    std:: transform(h.begin(), h.end(), h.begin(), ::tolower);
    std::transform(n.begin(), n.end(), n.begin(), ::tolower);
    return h.find(n) != std::string::npos;
}

// ============== SCORING FUNCTIONS ==============

// 1. SKILLS MATCHING - Jaccard Similarity with Bonus
double calculateSkillsScore(const std:: string& cvSkills, const std:: string& jobSkills) {
    std::set<std::string> cvSet = tokenizeSkills(cvSkills);
    std::set<std::string> jobSet = tokenizeSkills(jobSkills);
    
    if (jobSet.empty()) return 1.0; // No requirements
    
    // Count matching skills
    int matchCount = 0;
    for (const auto& jobSkill : jobSet) {
        for (const auto& cvSkill :  cvSet) {
            if (cvSkill == jobSkill || 
                cvSkill.find(jobSkill) != std::string::npos ||
                jobSkill.find(cvSkill) != std::string::npos) {
                matchCount++;
                break;
            }
        }
    }
    
    // Jaccard similarity
    double baseScore = (double)matchCount / jobSet.size();
    
    // BONUS: Extra skills beyond requirement (shows initiative)
    if (cvSet.size() > jobSet.size() && matchCount == jobSet.size()) {
        baseScore = std::min(1.0, baseScore + 0.1);
    }
    
    return baseScore;
}

// 2. EXPERIENCE SCORING - WITH COMPENSATION LOGIC
double calculateExperienceScore(int cvExp, int requiredExp) {
    if (requiredExp == 0) return 1.0;
    
    // PERFECT MATCH: Within range
    if (cvExp >= requiredExp && cvExp <= requiredExp + 3) {
        return 1.0;
    }
    
    // UNDER-QUALIFIED: Gradual penalty
    if (cvExp < requiredExp) {
        int gap = requiredExp - cvExp;
        
        // 1 year short: 0.85
        // 2 years short: 0.70
        // 3 years short: 0.55
        // 4+ years short: 0.40
        double score = 1.0 - (gap * 0.15);
        return std::max(0.40, score);  // Minimum 40% if any experience
    }
    
    // OVER-QUALIFIED: Slight penalty (might leave for better job)
    if (cvExp > requiredExp + 5) {
        return 0.85;
    }
    
    return 1.0;
}

// 3. EDUCATION SCORING - Degree Level Matching
double calculateEducationScore(const std::string& cvEdu, const std::string& jobEdu) {
    // Education hierarchy
    auto getLevel = [](const std::string& edu) -> int {
        std::string lower = edu;
        std::transform(lower. begin(), lower.end(), lower.begin(), ::tolower);
        
        if (lower. find("phd") != std::string::npos || lower.find("doctorate") != std::string::npos) return 4;
        if (lower.find("ms") != std::string::npos || lower.find("master") != std::string::npos) return 3;
        if (lower.find("bs") != std::string::npos || lower.find("bachelor") != std::string::npos) return 2;
        if (lower.find("diploma") != std::string::npos) return 1;
        return 0;
    };
    
    int cvLevel = getLevel(cvEdu);
    int jobLevel = getLevel(jobEdu);
    
    // Exact or higher qualification
    if (cvLevel >= jobLevel) {
        return 1.0;
    }
    
    // Under-qualified: 1 level down = 0.7, 2 levels = 0.4
    int gap = jobLevel - cvLevel;
    return std::max(0.3, 1.0 - (gap * 0.3));
}

// 4. POSITION MATCHING - Keyword-based
double calculatePositionScore(const std::string& cvPos, const std::string& jobPos) {
    // Exact match
    if (containsIgnoreCase(cvPos, jobPos) || containsIgnoreCase(jobPos, cvPos)) {
        return 1.0;
    }
    
    // Keyword matching
    std::set<std::string> keywords = {"engineer", "developer", "analyst", "manager", 
                                      "lead", "senior", "junior", "architect"};
    
    int matchCount = 0;
    for (const auto& keyword : keywords) {
        if (containsIgnoreCase(cvPos, keyword) && containsIgnoreCase(jobPos, keyword)) {
            matchCount++;
        }
    }
    
    return matchCount > 0 ? 0.7 : 0.3; // Partial credit
}

// 5. LOCATION SCORING
double calculateLocationScore(const std:: string& cvLoc, const std::string& jobLoc) {
    if (containsIgnoreCase(cvLoc, jobLoc)) return 1.0;
    return 0.5; // Willing to relocate possibility
}

// ============== MAIN MATCHING ENGINE ==============

DetailedScore matchCV(const CVRecord& cv, const JobRequirement& job) {
    DetailedScore result;
    result.cv = cv;
    
    // Calculate individual scores
    result.skillsScore = calculateSkillsScore(cv.skills, job.requiredSkills);
    result.experienceScore = calculateExperienceScore(cv.experience, job.minExperience);
    result.educationScore = calculateEducationScore(cv.education, job.education);
    result.positionScore = calculatePositionScore(cv.lastPosition, job.position);
    result.locationScore = calculateLocationScore(cv.location, job.location);
    
    // ====== WEIGHTED FINAL SCORE ======
    // Skills: 40% - Most important
    // Experience: 25% - Can be compensated
    // Education:  15% - Background matters
    // Position: 15% - Relevance
    // Location: 5% - Least important
    
    result.finalScore = 
        (result.skillsScore * 0.40) +
        (result.experienceScore * 0.25) +
        (result.educationScore * 0.15) +
        (result.positionScore * 0.15) +
        (result.locationScore * 0.05);
    
    return result;
}

// ============== RANKING FUNCTION ==============

std::vector<DetailedScore> findBestCVs(const std::vector<CVRecord>& cvs, 
                                       const JobRequirement& job,
                                       int topN = 10) {
    std::vector<DetailedScore> allScores;
    
    // Score all CVs
    for (const auto& cv : cvs) {
        if (cv.isDeleted) continue;
        
        DetailedScore score = matchCV(cv, job);
        
        // Only include if meets minimum threshold
        if (score.finalScore >= job.minAcceptableScore) {
            allScores.push_back(score);
        }
    }
    
    // Sort by final score (descending)
    std::sort(allScores.begin(), allScores.end(), 
              [](const DetailedScore& a, const DetailedScore& b) {
                  return a.finalScore > b.finalScore;
              });
    
    // Return top N
    if (allScores.size() > topN) {
        allScores. resize(topN);
    }
    
    return allScores;
}