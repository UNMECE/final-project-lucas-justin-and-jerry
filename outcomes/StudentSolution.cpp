#include "acequia_manager.h"
#include <iostream>
#include <algorithm> // for std::min/max

void solveProblems(AcequiaManager& manager) {
    auto canals = manager.getCanals();
    auto regions = manager.getRegions();
    
    // Target water levels as multipliers of waterNeed
    const double NORTH_TARGET = 1.25;  // North needs buffer above need
    const double SOUTH_TARGET = 1.0;   // South should meet exact need
    const double EAST_TARGET = 1.15;   // East needs slight buffer
    
    while(!manager.isSolved && manager.hour < manager.SimulationMax) {
        // Get current region states
        Region* north = nullptr;
        Region* south = nullptr;
        Region* east = nullptr;
        
        for(auto& region : regions) {
            if(region->name == "North") north = region;
            else if(region->name == "South") south = region;
            else if(region->name == "East") east = region;
        }
        
        // Close all canals initially
        for(auto& canal : canals) {
            canal->toggleOpen(false);
        }
        
        // 1. Handle North Region (typically needs water)
        if(north->waterLevel < north->waterNeed * NORTH_TARGET) {
            // Prioritize canals that can bring water to North
            for(auto& canal : canals) {
                if(canal->destinationRegion == north) {
                    double deficit = (north->waterNeed * NORTH_TARGET - north->waterLevel) / north->waterNeed;
                    // Dynamic flow rate based on need
                    double rate = std::min(0.3 + deficit * 0.5, 0.8);
                    canal->setFlowRate(rate);
                    canal->toggleOpen(true);
                }
            }
        }
        
        // 2. Handle South Region (often has excess water)
        if(south->waterLevel > south->waterNeed * SOUTH_TARGET) {
            // Open canals that can drain water from South
            for(auto& canal : canals) {
                if(canal->sourceRegion == south) {
                    double excess = (south->waterLevel - south->waterNeed * SOUTH_TARGET) / south->waterCapacity;
                    double rate = std::min(0.4 + excess * 0.4, 0.75);
                    canal->setFlowRate(rate);
                    canal->toggleOpen(true);
                }
            }
        }
        
        // 3. Handle East Region (needs careful balancing)
        if(east->waterLevel < east->waterNeed * 0.9) {
            // Need more water in East
            for(auto& canal : canals) {
                if(canal->destinationRegion == east) {
                    double need = (east->waterNeed - east->waterLevel) / east->waterNeed;
                    canal->setFlowRate(std::min(0.25 + need * 0.3, 0.6));
                    canal->toggleOpen(true);
                }
            }
        } else if(east->waterLevel > east->waterNeed * EAST_TARGET) {
            // East has too much water
            for(auto& canal : canals) {
                if(canal->sourceRegion == east) {
                    double excess = (east->waterLevel - east->waterNeed) / east->waterCapacity;
                    canal->setFlowRate(std::min(0.2 + excess * 0.3, 0.5));
                    canal->toggleOpen(true);
                }
            }
        }
        
        // 4. Emergency conditions override
        for(auto& region : regions) {
            // Flood prevention
            if(region->waterLevel > region->waterCapacity * 0.9) {
                for(auto& canal : canals) {
                    if(canal->sourceRegion == region) {
                        canal->setFlowRate(0.9);
                        canal->toggleOpen(true);
                    }
                }
            }
            // Drought prevention
            else if(region->waterLevel < region->waterNeed * 0.2) {
                for(auto& canal : canals) {
                    if(canal->destinationRegion == region) {
                        canal->setFlowRate(0.9);
                        canal->toggleOpen(true);
                    }
                }
            }
        }
        
        manager.nexthour();
    }
}