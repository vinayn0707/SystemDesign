#ifndef PARKING_TICKET_H
#define PARKING_TICKET_H

#include <string>
#include <chrono>
#include <memory>
#include "Vehicle.h"

class ParkingTicket {
private:
    std::string ticketNumber;
    std::chrono::system_clock::time_point entryTime;
    std::chrono::system_clock::time_point exitTime;
    double amountCharged;
    bool isPaid;
    std::shared_ptr<Vehicle> vehicle;
    int slotNumber;

public:
    ParkingTicket(const std::string& number, std::shared_ptr<Vehicle> vehicle, int slotNum);
    
    // Core functionality
    void markExit();
    double calculateFee(double hourlyRate);
    void processPayment(double amount);
    
    // Getters
    std::string getTicketNumber() const;
    bool isTicketPaid() const;
    std::string getVehicleLicenseNumber() const;
    int getSlotNumber() const;
    std::chrono::system_clock::duration getParkingDuration() const;
};

#endif // PARKING_TICKET_H
