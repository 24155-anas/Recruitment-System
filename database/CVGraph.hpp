#pragma once
#include <vector>
#include <string>
#include<algorithm>
#include <iostream>
#include "DataRecords.hpp"
using namespace std;
struct Node {
    int id;
    string label; // CV attribute like skills, last position
};

struct Edge {
    int from;
    int to;
    double weight; //hr edge ka koi weight bhi hoga
};

struct Graph {
    vector<Node> nodes;
    vector<Edge> edges;

    int addNode(const string& label) {
        int nodeId = nodes.size();
        nodes.push_back({nodeId, label});
        return nodeId;
    }
    void addEdge(int from, int to, double weight) {
        edges.push_back({from, to, weight});
        edges.push_back({to, from, weight}); //undirected graph

    }
    int findNodeByLabel(const string& label) {
        for (const auto& node : nodes) {
            if (node.label == label) {
                return node.id;
            }
        }
        return -1;
    }
};


// struct CVRecord {
//     int32_t cvId;
//     int32_t userId;  //foreign key
//     char name[50];
//     char email[50];
//     char skills[200];
//     int32_t experience;
//     char lastPosition[50];
//     char education[100];
//     char location[50];
//     bool isDeleted;  // Soft delete flag

Graph cvToGraph(const CVRecord& cv) {

    Graph g;
    int skillsNode = g.addNode(cv.skills);
    int expNode = g.addNode(std::to_string(cv.experience));
    int posNode = g.addNode(cv.lastPosition);
    int eduNode = g.addNode(cv.education);
    int locNode = g.addNode(cv.location);

    
    g.addEdge(skillsNode, expNode, 3.0);   // skills -> experience
    g.addEdge(skillsNode, posNode, 2.5);   // skills -> position
    g.addEdge(skillsNode, eduNode, 2.0);   // skills -> education
    g.addEdge(skillsNode, locNode, 1.0);   // skills -> location

    return g;
}

//humari job requirement to graph
Graph jobToGraph(const std::string& skills, int experience,
                 const std::string& education, const std::string& position,
                 const std::string& location) {
    Graph g;
    int skillsNode = g.addNode(skills);
    int expNode = g.addNode(std::to_string(experience));
    int posNode = g.addNode(position);
    int eduNode = g.addNode(education);
    int locNode = g.addNode(location);

    // Same weights as CV graph
    g.addEdge(skillsNode, expNode, 3.0);
    g.addEdge(skillsNode, posNode, 2.5);
    g.addEdge(skillsNode, eduNode, 2.0);
    g.addEdge(skillsNode, locNode, 1.0);

    return g;
}


double weightedGraphSimilarity(const Graph& cv, const Graph& job) {
    double totalWeight = 0;
    double matchedWeight = 0;

    // Calculate total weight of job graph
    for (const auto& e : job.edges)
        totalWeight += e.weight;

    // For each job edge, check if a matching CV edge exists
    for (const auto& je : job.edges) {
        const std::string& jFrom = job.nodes[je.from].label;
        const std::string& jTo = job.nodes[je.to].label;

        for (const auto& ce : cv.edges) {
            const std::string& cFrom = cv.nodes[ce.from].label;
            const std::string& cTo = cv.nodes[ce.to].label;

            if ((jFrom == cFrom && jTo == cTo) || (jFrom == cTo && jTo == cFrom)) {
                matchedWeight += je.weight;
                break; // count each job edge once
            }
        }
    }

    return totalWeight > 0 ? matchedWeight / totalWeight : 0.0;
}


struct CVScore {
    CVRecord cv;
    double score;
};

std::vector<CVScore> rankCVs(const std::vector<CVRecord>& cvs, const Graph& jobGraph) {
    std::vector<CVScore> results;
    for (const auto& cv : cvs) {
        Graph g = cvToGraph(cv);
        double s = weightedGraphSimilarity(g, jobGraph);
        results.push_back({cv, s});
    }

    std::sort(results.begin(), results.end(), [](const CVScore& a, const CVScore& b) {
        return a.score > b.score;
    });

    return results;
}

