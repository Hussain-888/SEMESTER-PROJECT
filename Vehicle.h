ifndef VEHICLE_H
#define VEHICLE_H

class Vehicle {
private:
    char* vehicleId;
    int preferredZone;

public:
    Vehicle(const char* vId, int prefZone);
    ~Vehicle();
    
    const char* getVehicleId() const;
    int getPreferredZone() const;
    void setPreferredZone(int zone);
};

#endif
