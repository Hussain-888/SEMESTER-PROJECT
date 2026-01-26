#include "ParkingSystem.h"
#include "Zone.h"
#include "ParkingArea.h"
#include "ParkingSlot.h"
#include "ParkingRequest.h"
#include "AllocationEngine.h"
#include "RollbackManager.h"
#include <iostream>
#include <ctime>

ParkingSystem::ParkingSystem(int maxZones) 
    : zoneCount(0), zoneCapacity(maxZones), requestHistoryHead(nullptr),
      requestHistoryTail(nullptr), nextRequestId(1), currentTime(0) {
    zones = new Zone*[maxZones];
    for (int i = 0; i < maxZones; i++) {
        zones[i] = nullptr;
    }
    engine = new AllocationEngine(zones, zoneCount);
    rollbackMgr = new RollbackManager();
}

ParkingSystem::~ParkingSystem() {
    for (int i = 0; i < zoneCount; i++) {
        delete zones[i];
    }
    delete[] zones;
    delete engine;
    delete rollbackMgr;
    
    RequestNode* current = requestHistoryHead;
    while (current != nullptr) {
        RequestNode* temp = current;
        current = current->next;
        delete temp->request;
        delete temp;
    }
}

bool ParkingSystem::addZone(Zone* zone) {
    if (zoneCount < zoneCapacity) {
        zones[zoneCount++] = zone;
        delete engine;
        engine = new AllocationEngine(zones, zoneCount);
        return true;
    }
    return false;
}

ParkingRequest* ParkingSystem::createRequest(const char* vehicleId, int requestedZone) {
    long long reqTime = getCurrentTime();
    ParkingRequest* request = new ParkingRequest(nextRequestId++, vehicleId, requestedZone, reqTime);
    addToHistory(request);
    
    // Automatic allocation
    if (engine->allocateSlot(request, reqTime)) {
        ParkingSlot* slot = nullptr;
        Zone* zone = getZone(request->getAllocatedZone());
        if (zone != nullptr) {
            for (int i = 0; i < zone->getAreaCount(); i++) {
                ParkingArea* area = zone->getParkingArea(i);
                for (int j = 0; j < area->getSlotCount(); j++) {
                    ParkingSlot* s = area->getSlot(j);
                    if (s->getSlotId() == request->getAllocatedSlotId()) {
                        slot = s;
                        break;
                    }
                }
                if (slot != nullptr) break;
            }
        }
        if (slot != nullptr) {
            rollbackMgr->pushAllocation(request, slot);
        }
        request->occupy(reqTime);
    }
    
    return request;
}

bool ParkingSystem::cancelRequest(int requestId) {
    ParkingRequest* request = findRequest(requestId);
    if (request == nullptr) return false;
    
    // Allow canceling if state is REQUESTED, ALLOCATED, or OCCUPIED
    // Do NOT allow if already RELEASED or CANCELLED
    RequestState state = request->getState();
    
    if (state == RELEASED || state == CANCELLED) {
        return false;  // Already released or cancelled
    }
    
    // Can cancel if REQUESTED, ALLOCATED, or OCCUPIED
    if (state == REQUESTED || state == ALLOCATED || state == OCCUPIED) {
        if (request->getAllocatedSlotId() != -1) {
            Zone* zone = getZone(request->getAllocatedZone());
            if (zone != nullptr) {
                for (int i = 0; i < zone->getAreaCount(); i++) {
                    ParkingArea* area = zone->getParkingArea(i);
                    for (int j = 0; j < area->getSlotCount(); j++) {
                        ParkingSlot* slot = area->getSlot(j);
                        if (slot->getSlotId() == request->getAllocatedSlotId()) {
                            slot->release();
                            break;
                        }
                    }
                }
            }
        }
        request->cancel();
        return true;
    }
    
    return false;
}

bool ParkingSystem::releaseParking(int requestId) {
    ParkingRequest* request = findRequest(requestId);
    if (request == nullptr) return false;
    
    if (request->getState() == OCCUPIED) {
        Zone* zone = getZone(request->getAllocatedZone());
        if (zone != nullptr) {
            for (int i = 0; i < zone->getAreaCount(); i++) {
                ParkingArea* area = zone->getParkingArea(i);
                for (int j = 0; j < area->getSlotCount(); j++) {
                    ParkingSlot* slot = area->getSlot(j);
                    if (slot->getSlotId() == request->getAllocatedSlotId()) {
                        slot->release();
                        break;
                    }
                }
            }
        }
        request->release(getCurrentTime());
        return true;
    }
    return false;
}

bool ParkingSystem::rollbackAllocations(int k) {
    return rollbackMgr->rollback(k);
}

void ParkingSystem::displayZoneStatus() const {
    std::cout << "\n=== Zone Status ===\n";
    for (int i = 0; i < zoneCount; i++) {
        Zone* zone = zones[i];
        std::cout << "Zone " << zone->getZoneId() << ": "
                  << zone->getAvailableSlots() << "/" << zone->getTotalSlots()
                  << " available";
        if (zone->isFull()) {
            std::cout << " [FULL]";
        }
        std::cout << "\n";
    }
}

void ParkingSystem::displayRequestHistory() const {
    std::cout << "\n=== Request History ===\n";
    RequestNode* current = requestHistoryHead;
    while (current != nullptr) {
        ParkingRequest* req = current->request;
        std::cout << "Request #" << req->getRequestId() 
                  << " | Vehicle: " << req->getVehicleId()
                  << " | Requested Zone: " << req->getRequestedZone()
                  << " | State: ";
        switch (req->getState()) {
            case REQUESTED: std::cout << "REQUESTED"; break;
            case ALLOCATED: std::cout << "ALLOCATED"; break;
            case OCCUPIED: std::cout << "OCCUPIED"; break;
            case RELEASED: std::cout << "RELEASED"; break;
            case CANCELLED: std::cout << "CANCELLED"; break;
        }
        if (req->getAllocatedSlotId() != -1) {
            std::cout << " | Slot: " << req->getAllocatedSlotId()
                      << " in Zone " << req->getAllocatedZone();
            if (req->hasCrossZonePenalty()) {
                std::cout << " [CROSS-ZONE PENALTY]";
            }
        }
        std::cout << "\n";
        current = current->next;
    }
}

void ParkingSystem::displayAnalytics() const {
    std::cout << "\n=== Analytics ===\n";
    
    int totalRequests = 0;
    int completedRequests = 0;
    int cancelledRequests = 0;
    long long totalDuration = 0;
    int zoneUsage[10] = {0};
    
    RequestNode* current = requestHistoryHead;
    while (current != nullptr) {
        ParkingRequest* req = current->request;
        totalRequests++;
        
        if (req->getState() == RELEASED) {
            completedRequests++;
            totalDuration += req->getParkingDuration();
        } else if (req->getState() == CANCELLED) {
            cancelledRequests++;
        }
        
        if (req->getAllocatedZone() >= 0 && req->getAllocatedZone() < 10) {
            zoneUsage[req->getAllocatedZone()]++;
        }
        
        current = current->next;
    }
    
    std::cout << "Total Requests: " << totalRequests << "\n";
    std::cout << "Completed: " << completedRequests << "\n";
    std::cout << "Cancelled: " << cancelledRequests << "\n";
    
    if (completedRequests > 0) {
        std::cout << "Average Parking Duration: " 
                  << (totalDuration / completedRequests) << " time units\n";
    }
    
    int peakZone = -1;
    int maxUsage = 0;
    for (int i = 0; i < zoneCount; i++) {
        int zoneId = zones[i]->getZoneId();
        if (zoneUsage[zoneId] > maxUsage) {
            maxUsage = zoneUsage[zoneId];
            peakZone = zoneId;
        }
        if (zones[i]->getTotalSlots() > 0) {
            double utilization = (double)(zones[i]->getTotalSlots() - zones[i]->getAvailableSlots()) 
                                / zones[i]->getTotalSlots() * 100.0;
            std::cout << "Zone " << zoneId << " Utilization: " 
                      << utilization << "%\n";
        }
    }
    
    if (peakZone != -1) {
        std::cout << "Peak Usage Zone: Zone " << peakZone 
                  << " (" << maxUsage << " allocations)\n";
    }
}

Zone* ParkingSystem::getZone(int zoneId) const {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i]->getZoneId() == zoneId) {
            return zones[i];
        }
    }
    return nullptr;
}

ParkingRequest* ParkingSystem::findRequest(int requestId) const {
    RequestNode* current = requestHistoryHead;
    while (current != nullptr) {
        if (current->request->getRequestId() == requestId) {
            return current->request;
        }
        current = current->next;
    }
    return nullptr;
}

void ParkingSystem::addToHistory(ParkingRequest* request) {
    RequestNode* newNode = new RequestNode(request);
    if (requestHistoryTail == nullptr) {
        requestHistoryHead = requestHistoryTail = newNode;
    } else {
        requestHistoryTail->next = newNode;
        requestHistoryTail = newNode;
    }
}

long long ParkingSystem::getCurrentTime() {
    return ++currentTime;
}
