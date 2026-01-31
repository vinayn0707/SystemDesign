#ifndef PARKING_SLOT_H
#define PARKING_SLOT_H

#include "Vehicle.h"
#include <memory>

class ParkingSlot {
private:
    int slotNumber;
    bool isOccupied;
    std::shared_ptr<Vehicle> parkedVehicle;
    VehicleType slotType;
    
public:
    ParkingSlot(int number, VehicleType type);
    
    // Core functionality
    bool isAvailable() const;
    bool canPark(VehicleType vehicleType) const;
    bool parkVehicle(std::shared_ptr<Vehicle> vehicle);
    void vacateSlot();
    
    // Getters
    int getSlotNumber() const;
    VehicleType getSlotType() const;
    std::shared_ptr<Vehicle> getParkedVehicle() const;
};

#endif // PARKING_SLOT_H
