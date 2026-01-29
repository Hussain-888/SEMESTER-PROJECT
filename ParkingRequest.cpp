#include "ParkingRequest.h"
#include <cstring>

ParkingRequest::ParkingRequest(int reqId, const char* vId, int reqZone, long long reqTime)
    : requestId(reqId), requestedZone(reqZone), allocatedZone(-1), 
      allocatedSlotId(-1), state(REQUESTED), requestTime(reqTime),
      allocationTime(0), releaseTime(0), crossZonePenalty(false) {
    vehicleId = new char[strlen(vId) + 1];
    strcpy(vehicleId, vId);
}

ParkingRequest::~ParkingRequest() {
    delete[] vehicleId;
}

int ParkingRequest::getRequestId() const {
    return requestId;
}

const char* ParkingRequest::getVehicleId() const {
    return vehicleId;
}

int ParkingRequest::getRequestedZone() const {
    return requestedZone;
}

int ParkingRequest::getAllocatedZone() const {
    return allocatedZone;
}

int ParkingRequest::getAllocatedSlotId() const {
    return allocatedSlotId;
}

RequestState ParkingRequest::getState() const {
    return state;
}

long long ParkingRequest::getRequestTime() const {
    return requestTime;
}

long long ParkingRequest::getAllocationTime() const {
    return allocationTime;
}

long long ParkingRequest::getReleaseTime() const {
    return releaseTime;
}

bool ParkingRequest::hasCrossZonePenalty() const {
    return crossZonePenalty;
}

bool ParkingRequest::transitionTo(RequestState newState) {
    // Valid transitions
    if (state == REQUESTED && (newState == ALLOCATED || newState == CANCELLED)) {
        state = newState;
        return true;
    }
    if (state == ALLOCATED && (newState == OCCUPIED || newState == CANCELLED)) {
        state = newState;
        return true;
    }
    if (state == OCCUPIED && newState == RELEASED) {
        state = newState;
        return true;
    }
    return false;
}

void ParkingRequest::allocate(int zoneId, int slotId, long long time, bool crossZone) {
    if (transitionTo(ALLOCATED)) {
        allocatedZone = zoneId;
        allocatedSlotId = slotId;
        allocationTime = time;
        crossZonePenalty = crossZone;
    }
}

void ParkingRequest::occupy(long long time) {
    transitionTo(OCCUPIED);
}

void ParkingRequest::release(long long time) {
    if (transitionTo(RELEASED)) {
        releaseTime = time;
    }
}

void ParkingRequest::cancel() {
    transitionTo(CANCELLED);
}

long long ParkingRequest::getParkingDuration() const {
    if (state == RELEASED && allocationTime > 0 && releaseTime > 0) {
        return releaseTime - allocationTime;
    }
    return 0;
}
