#include "ParkingSlot.h"

ParkingSlot::ParkingSlot(int sId, int zId) 
    : slotId(sId), zoneId(zId), available(true) {}

int ParkingSlot::getSlotId() const {
    return slotId;
}

int ParkingSlot::getZoneId() const {
    return zoneId;
}

bool ParkingSlot::isAvailable() const {
    return available;
}

void ParkingSlot::setAvailable(bool status) {
    available = status;
}

void ParkingSlot::occupy() {
    available = false;
}

void ParkingSlot::release() {
    available = true;
}
