#ifndef PARKING_LOT_H
#define PARKING_LOT_H

#include "ParkingSlot.h"
#include "ParkingTicket.h"
#include <vector>
#include <unordered_map>
#include <memory>

class ParkingLot {
private:
    std::vector<std::shared_ptr<ParkingSlot>> parkingSlots;
    std::unordered_map<std::string, std::shared_ptr<ParkingTicket>> activeTickets;
    double hourlyRate;
    int nextTicketNumber;
    
    std::string generateTicketNumber();
    
public:
    ParkingLot(int carSpaces, int truckSpaces, int motorcycleSpaces, double rate);
    
    // Core operations
    std::shared_ptr<ParkingTicket> parkVehicle(std::shared_ptr<Vehicle> vehicle);
    double exitParking(const std::string& ticketNumber);
    
    // Getters
    int getAvailableSpaces(VehicleType type) const;
    int getTotalSpaces() const;
    int getOccupiedSpaces() const;
    
    // Utility
    bool isFull() const;
    bool isFull(VehicleType type) const;
};

#endif // PARKING_LOT_H
