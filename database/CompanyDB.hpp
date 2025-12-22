#pragma once
#include "BlockManager.hpp"
#include "Btree.hpp"
#include "DataRecords.hpp"
#include <vector>
#include <string>

class CompanyDatabase {
private: 
    BlockManager* indexMgr;
    BlockManager* dataMgr;
    BTree* btree;
    int32_t nextCompanyId;
    int32_t currentDataBlock;
    
    int32_t allocateDataBlock() {
        int32_t blockNum = dataMgr->allocateBlock();
        
        char buffer[BLOCK_SIZE];
        memset(buffer, 0, BLOCK_SIZE);
        
        DataBlockHeader header;
        header.blockNumber = blockNum;
        header.recordCount = 0;
        header.freeSpace = BLOCK_SIZE - DataBlockHeader::size();
        header.nextBlock = -1;
        
        //block me header likh do
        memcpy(buffer, &header, DataBlockHeader::size());
        dataMgr->write(blockNum, buffer);
        
        return blockNum;
    }
    
public:
    CompanyDatabase(const std::string& indexFile, const std::string& dataFile) : nextCompanyId(1), currentDataBlock(-1) {
        
        indexMgr = new BlockManager(indexFile);
        dataMgr = new BlockManager(dataFile);
        btree = new BTree(indexMgr);
        
        //id k liye
        std::vector<CompanyRecord> existing = getAllCompanies();
        for (const auto& company : existing) {
            if (!company.isDeleted && company.companyId >= nextCompanyId) {
                nextCompanyId = company.companyId + 1;
            }
        }
        
        //peh;la block
        if (dataMgr->getRootBlock() == -1) {
            currentDataBlock = allocateDataBlock();
            dataMgr->setRootBlock(currentDataBlock);
        }
        //find last block in chain
        else {
            currentDataBlock = dataMgr->getRootBlock();
            char buffer[BLOCK_SIZE];
            
            while (true) {
                dataMgr->read(currentDataBlock, buffer);
                DataBlockHeader header;
                memcpy(&header, buffer, DataBlockHeader::size());
                
                if (header.nextBlock == -1) {
                    break;
                }
                currentDataBlock = header.nextBlock;
            }
        }
        
        std::cout << "CompanyDB ready. Next ID: " << nextCompanyId << std::endl;
    }
    
    ~CompanyDatabase() {
        delete btree;
        delete indexMgr;
        delete dataMgr;
    }
    
    //adding a company and its job posting
    int32_t addCompany(const std::string& companyName, const std::string& jobTitle,
                      const std::string& requiredSkills, int32_t minExperience,
                      const std::string& position, const std::string& education,
                      const std::string& location, int32_t salary,
                      const std::string& description) {
        
        CompanyRecord company;
        company.companyId = nextCompanyId++;
        company.minExperience = minExperience;
        company.salary = salary;
        company.isActive = true;
        company. isDeleted = false;
        company.postedDate = time(nullptr);
        
        strncpy(company.companyName, companyName.c_str(), sizeof(company.companyName) - 1);
        strncpy(company.jobTitle, jobTitle.c_str(), sizeof(company.jobTitle) - 1);
        strncpy(company.requiredSkills, requiredSkills.c_str(), sizeof(company.requiredSkills) - 1);
        strncpy(company.position, position.c_str(), sizeof(company.position) - 1);
        strncpy(company.education, education.c_str(), sizeof(company.education) - 1);
        strncpy(company.location, location.c_str(), sizeof(company.location) - 1);
        strncpy(company.description, description.c_str(), sizeof(company.description) - 1);
        
        //saving to disk
        char buffer[BLOCK_SIZE];
        dataMgr->read(currentDataBlock, buffer);
        
        DataBlockHeader header;
        memcpy(&header, buffer, DataBlockHeader::size());
        
        //agr current block me no askta
        if (header.freeSpace < CompanyRecord::size()) {
            int32_t newBlock = allocateDataBlock();
            header.nextBlock = newBlock;

            //block header update krdo
            memcpy(buffer, &header, DataBlockHeader:: size());
            dataMgr->write(currentDataBlock, buffer);
            
            currentDataBlock = newBlock;
            dataMgr->read(currentDataBlock, buffer);
            memcpy(&header, buffer, DataBlockHeader::size());
        }
        
        //calculate offset inside the block
        int32_t offset = DataBlockHeader::size() + (header.recordCount * CompanyRecord::size());
        memcpy(buffer + offset, &company, CompanyRecord::size());
        
        header.recordCount++;
        header.freeSpace -= CompanyRecord::size();
        memcpy(buffer, &header, DataBlockHeader::size());
        
        dataMgr->write(currentDataBlock, buffer);
        dataMgr->incrementRecordCount();
        
        //b tree me insert krdo
        IndexEntry entry(company.companyId, currentDataBlock, offset);
        btree->insert(entry);
        
        std::cout << "Company " << company.companyId << " added:  " << companyName << std:: endl;
        return company.companyId;
    }
    
    //find company by ccompany id
    CompanyRecord* getCompanyById(int32_t companyId) {
        IndexEntry entry;
        // b tree me search kro
        if (btree->search(companyId, entry)) {
            char buffer[BLOCK_SIZE];
            //wo wala block read krlo
            dataMgr->read(entry.blockNum, buffer);
            
            CompanyRecord* company = new CompanyRecord();
            memcpy(company, buffer + entry.offset, CompanyRecord::size());
            
            //check if deleted
            if (company->isDeleted) {
                delete company;
                return nullptr;
            }
            
            return company;
        }
        return nullptr;
    }
    
    //return all companies
    std::vector<CompanyRecord> getAllCompanies() {
        std::vector<CompanyRecord> companies;
        int32_t blockNum = dataMgr->getRootBlock();
        
        while (blockNum != -1) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(blockNum, buffer);
            
            DataBlockHeader header;
            memcpy(&header, buffer, DataBlockHeader::size());
            
            for (int i = 0; i < header.recordCount; i++) {
                int32_t offset = DataBlockHeader::size() + (i * CompanyRecord::size());
                CompanyRecord company;
                memcpy(&company, buffer + offset, CompanyRecord:: size());
                
                if (!company.isDeleted) {
                    companies.push_back(company);
                }
            }
            
            blockNum = header.nextBlock;
        }
        
        return companies;
    }
    
    //return active companies only
    std::vector<CompanyRecord> getActiveCompanies() {
        std::vector<CompanyRecord> allCompanies = getAllCompanies();
        std::vector<CompanyRecord> activeCompanies;
        
        for (const auto& company : allCompanies) {
            if (company.isActive && !company.isDeleted) {
                activeCompanies.push_back(company);
            }
        }
        
        return activeCompanies;
    }

    //deleting company (soft delete)
    bool deleteCompany(int32_t companyId) {
        IndexEntry entry;
        if (btree->search(companyId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry. blockNum, buffer);
            
            CompanyRecord company;
            memcpy(&company, buffer + entry.offset, CompanyRecord::size());
            company.isDeleted = true; //mark deleted
            
            memcpy(buffer + entry.offset, &company, CompanyRecord::size());
            dataMgr->write(entry. blockNum, buffer);
            
            btree->remove(companyId); //remove `from b tree
            
            std::cout << "Company " << companyId << " deleted" << std::endl;
            return true;
        }
        return false;
    }
    
    //change active status
    bool toggleActiveStatus(int32_t companyId, bool isActive) {
        IndexEntry entry;
        if (btree->search(companyId, entry)) {
            char buffer[BLOCK_SIZE];
            dataMgr->read(entry. blockNum, buffer);
            
            CompanyRecord company;
            memcpy(&company, buffer + entry.offset, CompanyRecord:: size());
            company.isActive = isActive;
            
            memcpy(buffer + entry. offset, &company, CompanyRecord::size());
            dataMgr->write(entry.blockNum, buffer);
            
            std::cout << "Company " << companyId << " active status:  " << isActive << std::endl;
            return true;
        }
        return false;
    }
};

