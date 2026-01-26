#include "../include/ParkingLot.h"
#include <sstream>
#include <iomanip>
#include <algorithm>

ParkingLot::ParkingLot(int carSpaces, int truckSpaces, int motorcycleSpaces, double rate)
    : hourlyRate(rate), nextTicketNumber(1) {
    
    // Initialize parking slots
    int slotNumber = 1;
    
    // Add truck slots
    for (int i = 0; i < truckSpaces; ++i) {
        parkingSlots.push_back(std::make_shared<ParkingSlot>(slotNumber++, VehicleType::TRUCK));
    }
    
    // Add car slots
    for (int i = 0; i < carSpaces; ++i) {
        parkingSlots.push_back(std::make_shared<ParkingSlot>(slotNumber++, VehicleType::CAR));
    }
    
    // Add motorcycle slots
    for (int i = 0; i < motorcycleSpaces; ++i) {
        parkingSlots.push_back(std::make_shared<ParkingSlot>(slotNumber++, VehicleType::MOTORCYCLE));
    }
}

std::string ParkingLot::generateTicketNumber() {
    std::stringstream ss;
    ss << "TKT" << std::setw(8) << std::setfill('0') << nextTicketNumber++;
    return ss.str();
}

std::shared_ptr<ParkingTicket> ParkingLot::parkVehicle(std::shared_ptr<Vehicle> vehicle) {
    if (!vehicle) return nullptr;
    
    // Find first available slot for the vehicle type
    auto it = std::find_if(parkingSlots.begin(), parkingSlots.end(),
        [&vehicle](const std::shared_ptr<ParkingSlot>& slot) {
            return slot->canPark(vehicle->getType());
        });
    
    if (it == parkingSlots.end()) {
        return nullptr; // No available slot
    }
    
    auto slot = *it;
    if (slot->parkVehicle(vehicle)) {
        std::string ticketNumber = generateTicketNumber();
        auto ticket = std::make_shared<ParkingTicket>(ticketNumber, vehicle, slot->getSlotNumber());
        activeTickets[ticketNumber] = ticket;
        return ticket;
    }
    
    return nullptr;
}

double ParkingLot::exitParking(const std::string& ticketNumber) {
    auto it = activeTickets.find(ticketNumber);
    if (it == activeTickets.end()) {
        return -1.0; // Invalid ticket
    }
    
    auto ticket = it->second;
    double fee = ticket->calculateFee(hourlyRate);
    
    // Free up the parking slot
    int slotNum = ticket->getSlotNumber();
    if (slotNum > 0 && slotNum <= parkingSlots.size()) {
        parkingSlots[slotNum - 1]->vacateSlot();
    }
    
    // Remove from active tickets
    activeTickets.erase(it);
    
    return fee;
}

int ParkingLot::getAvailableSpaces(VehicleType type) const {
    return std::count_if(parkingSlots.begin(), parkingSlots.end(),
        [type](const std::shared_ptr<ParkingSlot>& slot) {
            return slot->isAvailable() && 
                  (slot->getSlotType() == type || 
                   (type == VehicleType::CAR && slot->getSlotType() == VehicleType::TRUCK));
        });
}

int ParkingLot::getTotalSpaces() const {
    return static_cast<int>(parkingSlots.size());
}

int ParkingLot::getOccupiedSpaces() const {
    return getTotalSpaces() - 
           std::count_if(parkingSlots.begin(), parkingSlots.end(),
               [](const std::shared_ptr<ParkingSlot>& slot) {
                   return slot->isAvailable();
               });
}

bool ParkingLot::isFull() const {
    return std::none_of(parkingSlots.begin(), parkingSlots.end(),
        [](const std::shared_ptr<ParkingSlot>& slot) {
            return slot->isAvailable();
        });
}

bool ParkingLot::isFull(VehicleType type) const {
    return getAvailableSpaces(type) == 0;
}
