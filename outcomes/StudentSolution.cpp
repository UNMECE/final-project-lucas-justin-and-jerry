#include "acequia_manager.h"
#include <iostream>
#include <vector>
#include <algorithm>

// Helper function to find canals connected to a specific region
std::vector<Canal*> findConnectedCanals(const std::vector<Canal*>& canals, 
                                       const std::string& regionName, 
                                       bool asSource) {
    std::vector<Canal*> connected;
    for (auto canal : canals) {
        if ((asSource && canal->sourceRegion->name == regionName) || 
            (!asSource && canal->destinationRegion->name == regionName)) {
            connected.push_back(canal);
        }
    }
    return connected;
}

// Function to balance water between regions
void balanceWater(AcequiaManager& manager) {
    auto regions = manager.getRegions();  // Fixed typo from getRegions() to getRegions()
    auto canals = manager.getCanals();
    
    for (auto region : regions) {
        // Check if region is in drought
        if (region->isInDrought) {
            auto incomingCanals = findConnectedCanals(canals, region->name, false);
            
            for (auto canal : incomingCanals) {
                if (!canal->isOpen) {
                    canal->toggleOpen(true);
                    // Set flow rate based on severity of drought
                    double rate = std::min(1.0, (region->waterNeed - region->waterLevel) / 10.0);
                    canal->setFlowRate(rate);
                }
            }
        }
        
        // Check if region is flooded
        if (region->isFlooded) {
            auto outgoingCanals = findConnectedCanals(canals, region->name, true);
            
            for (auto canal : outgoingCanals) {
                if (!canal->isOpen) {
                    canal->toggleOpen(true);
                    // Set flow rate based on severity of flood
                    double rate = std::min(1.0, (region->waterLevel - region->waterNeed) / 10.0);
                    canal->setFlowRate(rate);
                }
            }
        }
    }
}

// Function to check if all regions are in good state
bool checkSolution(const std::vector<Region*>& regions) {
    for (auto region : regions) {
        if (region->isFlooded || region->isInDrought || 
            region->waterLevel < region->waterNeed * 0.95 || 
            region->waterLevel > region->waterNeed * 1.05) {
            return false;
        }
    }
    return true;
}

void solveProblems(AcequiaManager& manager) {
    auto canals = manager.getCanals();
    auto regions = manager.getRegions();
    
    while (!manager.isSolved && manager.hour != manager.SimulationMax) {
        // Perform water balancing
        balanceWater(manager);
        
        // Check if we've solved the problem
        if (checkSolution(regions)) {
            manager.isSolved = true;
            break;
        }
        
        // Close any unnecessary canals at regular intervals
        if (manager.hour % 5 == 0) {
            for (auto canal : canals) {
                if (canal->isOpen && canal->flowRate < 0.2) {
                    canal->toggleOpen(false);
                }
            }
        }
        
        manager.nexthour();
    }
}