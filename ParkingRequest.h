#ifndef PARKINGREQUEST_H
#define PARKINGREQUEST_H

enum RequestState {
    REQUESTED,
    ALLOCATED,
    OCCUPIED,
    RELEASED,
    CANCELLED
};

class ParkingRequest {
private:
    int requestId;
    char* vehicleId;
    int requestedZone;
    int allocatedZone;
    int allocatedSlotId;
    RequestState state;
    long long requestTime;
    long long allocationTime;
    long long releaseTime;
    bool crossZonePenalty;

public:
    ParkingRequest(int reqId, const char* vId, int reqZone, long long reqTime);
    ~ParkingRequest();
    
    int getRequestId() const;
    const char* getVehicleId() const;
    int getRequestedZone() const;
    int getAllocatedZone() const;
    int getAllocatedSlotId() const;
    RequestState getState() const;
    long long getRequestTime() const;
    long long getAllocationTime() const;
    long long getReleaseTime() const;
    bool hasCrossZonePenalty() const;
    
    bool transitionTo(RequestState newState);
    void allocate(int zoneId, int slotId, long long time, bool crossZone);
    void occupy(long long time);
    void release(long long time);
    void cancel();
    long long getParkingDuration() const;
};

#endif
