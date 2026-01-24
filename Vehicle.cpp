#include "Vehicle.h"
#include <cstring>

Vehicle::Vehicle(const char* vId, int prefZone) : preferredZone(prefZone) {
    vehicleId = new char[strlen(vId) + 1];
    strcpy(vehicleId, vId);
}

Vehicle::~Vehicle() {
    delete[] vehicleId;
}

const char* Vehicle::getVehicleId() const {
    return vehicleId;
}

int Vehicle::getPreferredZone() const {
    return preferredZone;
}

void Vehicle::setPreferredZone(int zone) {
    preferredZone = zone;
}
