#ifndef VEHICLE_H
#define VEHICLE_H

#include <string>

enum class VehicleType {
    CAR,
    TRUCK,
    MOTORCYCLE,
    ELECTRIC
};

class Vehicle {
private:
    std::string licenseNumber;
    VehicleType type;

public:
    Vehicle(const std::string& licenseNum, VehicleType vehicleType);
    
    // Getters
    std::string getLicenseNumber() const;
    VehicleType getType() const;
};

#endif // VEHICLE_H
