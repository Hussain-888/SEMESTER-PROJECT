#include "AllocationEngine.h"
#include "Zone.h"
#include "ParkingArea.h"
#include "ParkingSlot.h"
#include "ParkingRequest.h"

AllocationEngine::AllocationEngine(Zone** zs, int count) 
    : zones(zs), zoneCount(count) {}

bool AllocationEngine::allocateSlot(ParkingRequest* request, long long currentTime) {
    int requestedZone = request->getRequestedZone();
    
    // Try same-zone allocation first
    ParkingSlot* slot = findSlotInZone(requestedZone);
    if (slot != nullptr) {
        slot->occupy();
        request->allocate(requestedZone, slot->getSlotId(), currentTime, false);
        return true;
    }
    
    // Try cross-zone allocation
    int foundZoneId = -1;
    slot = findSlotInOtherZones(requestedZone, foundZoneId);
    if (slot != nullptr) {
        slot->occupy();
        request->allocate(foundZoneId, slot->getSlotId(), currentTime, true);
        return true;
    }
    
    return false;
}

ParkingSlot* AllocationEngine::findSlotInZone(int zoneId) const {
    Zone* zone = getZone(zoneId);
    if (zone == nullptr) return nullptr;
    
    for (int i = 0; i < zone->getAreaCount(); i++) {
        ParkingArea* area = zone->getParkingArea(i);
        ParkingSlot* slot = area->findAvailableSlot();
        if (slot != nullptr) {
            return slot;
        }
    }
    return nullptr;
}

ParkingSlot* AllocationEngine::findSlotInOtherZones(int excludeZoneId, int& foundZoneId) const {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i]->getZoneId() == excludeZoneId) continue;
        
        ParkingSlot* slot = findSlotInZone(zones[i]->getZoneId());
        if (slot != nullptr) {
            foundZoneId = zones[i]->getZoneId();
            return slot;
        }
    }
    return nullptr;
}

Zone* AllocationEngine::getZone(int zoneId) const {
    for (int i = 0; i < zoneCount; i++) {
        if (zones[i]->getZoneId() == zoneId) {
            return zones[i];
        }
    }
    return nullptr;
}
