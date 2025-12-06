#include "database/CandidateDB.hpp"
#include <iostream>

int main() {
    std::cout << "=== Testing B-tree Database for Recruitment System ===\n" << std::endl;
    
    // Create/Open database
    CandidateDB db("index.dat", "candidates.dat");
    
    // Insert test data
    std::cout << "1. Inserting test candidates..." << std::endl;
    db.insertTestData();
    
    // Print index structure
    db.printIndex();
    
    // Test searching
    std::cout << "\n2. Testing search functionality..." << std::endl;
    
    std::cout << "\nSearching for ID 1:" << std::endl;
    CandidateRecord* result = db.searchById(1);
    if (result) {
        result->print();
        delete result;
    } else {
        std::cout << "Not found!" << std::endl;
    }
    
    std::cout << "\nSearching for ID 3:" << std::endl;
    result = db.searchById(3);
    if (result) {
        result->print();
        delete result;
    } else {
        std::cout << "Not found!" << std::endl;
    }
    
    std::cout << "\nSearching for non-existent ID 99:" << std::endl;
    result = db.searchById(99);
    if (result) {
        result->print();
        delete result;
    } else {
        std::cout << "Not found! (Expected)" << std::endl;
    }
    
    // Insert more candidates to trigger B-tree splitting
    std::cout << "\n3. Inserting more candidates to test B-tree splitting..." << std::endl;
    
    CandidateRecord moreCandidates[] = {
        {0, "David Lee", "david@email.com", "C++,Embedded Systems", 8, "Embedded Engineer"},
        {0, "Emma Davis", "emma@email.com", "Python,Django,PostgreSQL", 6, "Fullstack Dev"},
        {0, "Frank Miller", "frank@email.com", "Java,Spring,Hibernate", 5, "Backend Dev"},
        {0, "Grace Taylor", "grace@email.com", "React,Redux,TypeScript", 4, "UI Developer"},
        {0, "Henry Clark", "henry@email.com", "AWS,Docker,Kubernetes", 7, "Cloud Engineer"}
    };
    
    for (int i = 0; i < 5; i++) {
        db.addCandidate(moreCandidates[i]);
    }
    
    // Print updated index
    db.printIndex();
    
    // Print all records
    db.printAllRecords();
    
    std::cout << "\n=== Database Test Complete ===" << std::endl;
    std::cout << "\nFiles created:" << std::endl;
    std::cout << "- index.dat (B-tree index file)" << std::endl;
    std::cout << "- candidates.dat (Data records file)" << std::endl;
    
    return 0;
}