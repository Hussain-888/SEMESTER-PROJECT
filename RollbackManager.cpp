#include "RollbackManager.h"
#include "ParkingRequest.h"
#include "ParkingSlot.h"

RollbackManager::RollbackManager() : top(nullptr), stackSize(0) {}

RollbackManager::~RollbackManager() {
    clear();
}

void RollbackManager::pushAllocation(ParkingRequest* request, ParkingSlot* slot) {
    AllocationRecord* newRecord = new AllocationRecord(request, slot);
    newRecord->next = top;
    top = newRecord;
    stackSize++;
}

bool RollbackManager::rollback(int k) {
    if (k <= 0 || k > stackSize) return false;
    
    for (int i = 0; i < k; i++) {
        if (top == nullptr) return false;
        
        AllocationRecord* record = top;
        
        // Restore slot availability
        if (record->slot != nullptr) {
            record->slot->release();
        }
        
        // Cancel the request
        if (record->request != nullptr) {
            record->request->cancel();
        }
        
        top = top->next;
        delete record;
        stackSize--;
    }
    
    return true;
}

int RollbackManager::getStackSize() const {
    return stackSize;
}

void RollbackManager::clear() {
    while (top != nullptr) {
        AllocationRecord* temp = top;
        top = top->next;
        delete temp;
    }
    stackSize = 0;
}
