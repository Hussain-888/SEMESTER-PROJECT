#ifndef PARKINGAREA_H
#define PARKINGAREA_H

class ParkingSlot;

class ParkingArea {
private:
    int areaId;
    int zoneId;
    ParkingSlot** slots;
    int slotCount;
    int slotCapacity;

public:
    ParkingArea(int aId, int zId, int maxSlots);
    ~ParkingArea();
    
    int getAreaId() const;
    int getZoneId() const;
    bool addSlot(ParkingSlot* slot);
    ParkingSlot* getSlot(int index) const;
    ParkingSlot* findAvailableSlot() const;
    int getSlotCount() const;
    int getTotalSlots() const;
    int getAvailableSlots() const;
};

#endif
