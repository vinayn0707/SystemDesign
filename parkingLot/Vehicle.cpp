#include "../include/Vehicle.h"

Vehicle::Vehicle(const std::string& licenseNum, VehicleType vehicleType)
    : licenseNumber(licenseNum), type(vehicleType) {}

std::string Vehicle::getLicenseNumber() const {
    return licenseNumber;
}

VehicleType Vehicle::getType() const {
    return type;
}
