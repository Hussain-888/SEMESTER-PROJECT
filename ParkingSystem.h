#ifndef PARKINGSYSTEM_H
#define PARKINGSYSTEM_H

class Zone;
class ParkingRequest;
class AllocationEngine;
class RollbackManager;

struct RequestNode {
    ParkingRequest* request;
    RequestNode* next;
    
    RequestNode(ParkingRequest* req) : request(req), next(nullptr) {}
};

class ParkingSystem {
private:
    Zone** zones;
    int zoneCount;
    int zoneCapacity;
    AllocationEngine* engine;
    RollbackManager* rollbackMgr;
    RequestNode* requestHistoryHead;
    RequestNode* requestHistoryTail;
    int nextRequestId;
    long long currentTime;

public:
    ParkingSystem(int maxZones);
    ~ParkingSystem();
    
    bool addZone(Zone* zone);
    ParkingRequest* createRequest(const char* vehicleId, int requestedZone);
    bool cancelRequest(int requestId);
    bool releaseParking(int requestId);
    bool rollbackAllocations(int k);
    
    void displayZoneStatus() const;
    void displayRequestHistory() const;
    void displayAnalytics() const;
    
    Zone* getZone(int zoneId) const;
    ParkingRequest* findRequest(int requestId) const;
    
private:
    void addToHistory(ParkingRequest* request);
    long long getCurrentTime();
};

#endif
