#ifndef CANDIDATEDB_H
#define CANDIDATEDB_H

#include "Btree.hpp"
#include <fstream>
#include <iostream>
#include <string>

struct CandidateRecord {
    int32_t id;
    char name[50];
    char email[50];
    char skills[200];
    int experience;
    char lastPosition[50];
    
    CandidateRecord() : id(-1), experience(0) {
        memset(name, 0, sizeof(name));
        memset(email, 0, sizeof(email));
        memset(skills, 0, sizeof(skills));
        memset(lastPosition, 0, sizeof(lastPosition));
    }
    CandidateRecord(int32_t id_, const char* name_, const char* email_, 
                   const char* skills_, int exp_, const char* lastPos_) 
        : id(id_), experience(exp_) {
        
        strncpy(name, name_, sizeof(name)-1);
        name[sizeof(name)-1] = '\0';
        
        strncpy(email, email_, sizeof(email)-1);
        email[sizeof(email)-1] = '\0';
        
        strncpy(skills, skills_, sizeof(skills)-1);
        skills[sizeof(skills)-1] = '\0';
        
        strncpy(lastPosition, lastPos_, sizeof(lastPosition)-1);
        lastPosition[sizeof(lastPosition)-1] = '\0';
    }
    void print() const {
        std::cout << "ID: " << id << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Email: " << email << std::endl;
        std::cout << "Skills: " << skills << std::endl;
        std::cout << "Experience: " << experience << " years" << std::endl;
        std::cout << "Last Position: " << lastPosition << std::endl;
        std::cout << "------------------------" << std::endl;
    }
};

class CandidateDB {
private:
    BTree* indexTree;
    std::fstream dataFile;
    std::string dataFilename;
    int32_t nextId;
    
public:
    CandidateDB(const std::string& indexFile, const std::string& dataFile)
        : dataFilename(dataFile), nextId(1) {
        
        indexTree = new BTree(indexFile);
        
        // Open data file
        this->dataFile.open(dataFilename, 
            std::ios::in | std::ios::out | std::ios::binary | std::ios::app);
        
        if (!this->dataFile.is_open()) {
            this->dataFile.open(dataFilename, std::ios::out | std::ios::binary);
            this->dataFile.close();
            this->dataFile.open(dataFilename, 
                std::ios::in | std::ios::out | std::ios::binary);
        }
        
        // Find next ID from existing data
        this->dataFile.seekg(0, std::ios::end);
        std::streampos size = this->dataFile.tellg();
        if (size > 0) {
            nextId = (size / sizeof(CandidateRecord)) + 1;
        }
    }
    
    ~CandidateDB() {
        dataFile.close();
        delete indexTree;
    }
    
    int32_t addCandidate(const CandidateRecord& candidate) {
        // Assign ID
        CandidateRecord candidateWithId = candidate;
        candidateWithId.id = nextId++;
        
        // Write to data file
        dataFile.seekp(0, std::ios::end);
        int64_t offset = dataFile.tellp();
        
        dataFile.write(reinterpret_cast<const char*>(&candidateWithId), 
                      sizeof(CandidateRecord));
        dataFile.flush();
        
        // Add to B-tree index
        BTreeNode::IndexEntry entry(candidateWithId.id, offset, sizeof(CandidateRecord));
        indexTree->insert(entry);
        
        std::cout << "Added candidate ID " << candidateWithId.id 
                  << " at offset " << offset << std::endl;
        
        return candidateWithId.id;
    }
    
    CandidateRecord* searchById(int32_t id) {
        BTreeNode::IndexEntry entry;
        if (indexTree->search(id, entry)) {
            dataFile.seekg(entry.dataOffset);
            CandidateRecord* record = new CandidateRecord();
            dataFile.read(reinterpret_cast<char*>(record), sizeof(CandidateRecord));
            return record;
        }
        return nullptr;
    }
    
    void printIndex() {
        std::cout << "\n=== B-Tree Index Structure ===" << std::endl;
        indexTree->printTree();
    }
    
    void printAllRecords() {
        std::cout << "\n=== All Candidate Records ===" << std::endl;
        dataFile.seekg(0);
        
        while (dataFile) {
            CandidateRecord record;
            dataFile.read(reinterpret_cast<char*>(&record), sizeof(CandidateRecord));
            
            if (dataFile.gcount() == sizeof(CandidateRecord)) {
                record.print();
            }
        }
    }
    
    // Test function to insert dummy data
    void insertTestData() {
        CandidateRecord candidates[] = {
            {0, "John Doe", "john@email.com", "C++,Python,SQL", 5, "Software Engineer"},
            {0, "Jane Smith", "jane@email.com", "Java,JavaScript,React", 3, "Frontend Dev"},
            {0, "Bob Johnson", "bob@email.com", "C++,Azure,Docker", 7, "DevOps Engineer"},
            {0, "Alice Brown", "alice@email.com", "Python,ML,SQL", 4, "Data Scientist"},
            {0, "Charlie Wilson", "charlie@email.com", "JavaScript,Node.js", 2, "Backend Dev"}
        };
        
        for (int i = 0; i < 5; i++) {
            addCandidate(candidates[i]);
        }
    }
};
#endif