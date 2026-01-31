#include "../include/ParkingTicket.h"

ParkingTicket::ParkingTicket(const std::string& number, std::shared_ptr<Vehicle> vehicle, int slotNum)
    : ticketNumber(number), vehicle(vehicle), slotNumber(slotNum), 
      entryTime(std::chrono::system_clock::now()), isPaid(false) {}

void ParkingTicket::markExit() {
    exitTime = std::chrono::system_clock::now();
}

double ParkingTicket::calculateFee(double hourlyRate) {
    auto duration = std::chrono::duration_cast<std::chrono::hours>(
        std::chrono::system_clock::now() - entryTime);
    double hours = duration.count() + 1; // Round up to the next hour
    amountCharged = hours * hourlyRate;
    return amountCharged;
}

void ParkingTicket::processPayment(double amount) {
    if (amount >= amountCharged) {
        isPaid = true;
        markExit();
    }
}

std::string ParkingTicket::getTicketNumber() const {
    return ticketNumber;
}

bool ParkingTicket::isTicketPaid() const {
    return isPaid;
}

std::string ParkingTicket::getVehicleLicenseNumber() const {
    return vehicle ? vehicle->getLicenseNumber() : "";
}

int ParkingTicket::getSlotNumber() const {
    return slotNumber;
}

std::chrono::system_clock::duration ParkingTicket::getParkingDuration() const {
    return exitTime - entryTime;
}
