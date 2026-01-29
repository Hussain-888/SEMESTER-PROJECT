#ifndef ALLOCATIONENGINE_H
#define ALLOCATIONENGINE_H

class Zone;
class ParkingSlot;
class ParkingRequest;

class AllocationEngine {
private:
    Zone** zones;
    int zoneCount;

public:
    AllocationEngine(Zone** zs, int count);
    
    bool allocateSlot(ParkingRequest* request, long long currentTime);
    ParkingSlot* findSlotInZone(int zoneId) const;
    ParkingSlot* findSlotInOtherZones(int excludeZoneId, int& foundZoneId) const;
    Zone* getZone(int zoneId) const;
};

#endif
