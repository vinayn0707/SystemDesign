#include "../include/ParkingSlot.h"

ParkingSlot::ParkingSlot(int number, VehicleType type)
    : slotNumber(number), isOccupied(false), slotType(type) {}

bool ParkingSlot::isAvailable() const {
    return !isOccupied;
}

bool ParkingSlot::canPark(VehicleType vehicleType) const {
    return !isOccupied && (slotType == vehicleType || 
                          (slotType == VehicleType::TRUCK && vehicleType == VehicleType::CAR));
}

bool ParkingSlot::parkVehicle(std::shared_ptr<Vehicle> vehicle) {
    if (!isOccupied && (slotType == vehicle->getType() || 
                       (slotType == VehicleType::TRUCK && vehicle->getType() == VehicleType::CAR))) {
        parkedVehicle = vehicle;
        isOccupied = true;
        return true;
    }
    return false;
}

void ParkingSlot::vacateSlot() {
    parkedVehicle = nullptr;
    isOccupied = false;
}

int ParkingSlot::getSlotNumber() const {
    return slotNumber;
}

VehicleType ParkingSlot::getSlotType() const {
    return slotType;
}

std::shared_ptr<Vehicle> ParkingSlot::getParkedVehicle() const {
    return parkedVehicle;
}
