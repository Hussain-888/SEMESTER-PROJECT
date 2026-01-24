#ifndef ROLLBACKMANAGER_H
#define ROLLBACKMANAGER_H

class ParkingRequest;
class ParkingSlot;

struct AllocationRecord {
    ParkingRequest* request;
    ParkingSlot* slot;
    AllocationRecord* next;
    
    AllocationRecord(ParkingRequest* req, ParkingSlot* s) 
        : request(req), slot(s), next(nullptr) {}
};

class RollbackManager {
private:
    AllocationRecord* top;
    int stackSize;

public:
    RollbackManager();
    ~RollbackManager();
    
    void pushAllocation(ParkingRequest* request, ParkingSlot* slot);
    bool rollback(int k);
    int getStackSize() const;
    void clear();
};

#endif
