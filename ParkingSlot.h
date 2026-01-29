#ifndef PARKINGSLOT_H
#define PARKINGSLOT_H

class ParkingSlot {
private:
    int slotId;
    int zoneId;
    bool available;

public:
    ParkingSlot(int sId, int zId);
    
    int getSlotId() const;
    int getZoneId() const;
    bool isAvailable() const;
    void setAvailable(bool status);
    void occupy();
    void release();
};

#endif
