#include "Zone.h"
#include "ParkingArea.h"

Zone::Zone(int id, int maxAreas) : zoneId(id), areaCount(0), areaCapacity(maxAreas) {
    parkingAreas = new ParkingArea*[maxAreas];
    for (int i = 0; i < maxAreas; i++) {
        parkingAreas[i] = nullptr;
    }
}

Zone::~Zone() {
    for (int i = 0; i < areaCount; i++) {
        delete parkingAreas[i];
    }
    delete[] parkingAreas;
}

int Zone::getZoneId() const {
    return zoneId;
}

bool Zone::addParkingArea(ParkingArea* area) {
    if (areaCount < areaCapacity) {
        parkingAreas[areaCount++] = area;
        return true;
    }
    return false;
}

ParkingArea* Zone::getParkingArea(int index) const {
    if (index >= 0 && index < areaCount) {
        return parkingAreas[index];
    }
    return nullptr;
};;;;;

int Zone::getAreaCount() const {
    return areaCount;
}

int Zone::getTotalSlots() const {
    int total = 0;
    for (int i = 0; i < areaCount; i++) {
        total += parkingAreas[i]->getTotalSlots();
    }
    return total;
}
