#ifndef ZONE_H
#define ZONE_H

class ParkingArea;

class Zone {
private:
    int zoneId;
    ParkingArea** parkingAreas;
    int areaCount;
    int areaCapacity;

public:
    Zone(int id, int maxAreas);
    ~Zone();
    
    int getZoneId() const;
    bool addParkingArea(ParkingArea* area);
    ParkingArea* getParkingArea(int index) const;
    int getAreaCount() const;
    int getTotalSlots() const;
    int getAvailableSlots() const;
    bool isFull() const;
};

#endif
