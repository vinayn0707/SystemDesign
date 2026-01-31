#include "../include/Payment.h"
#include <sstream>
#include <iomanip>

Payment::Payment(std::shared_ptr<ParkingTicket> parkingTicket)
    : ticket(parkingTicket), isCompleted(false) {
    // Generate a simple payment ID (in a real system, this would be more sophisticated)
    std::stringstream ss;
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    ss << "PAY" << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    paymentId = ss.str();
}

bool Payment::processPayment(double amount) {
    if (!ticket) return false;
    
    if (amount >= ticket->calculateFee(10.0)) { // Assuming $10/hour rate for this example
        this->amount = amount;
        paymentTime = std::chrono::system_clock::now();
        ticket->processPayment(amount);
        isCompleted = true;
        return true;
    }
    return false;
}

std::string Payment::getPaymentId() const {
    return paymentId;
}

double Payment::getAmount() const {
    return amount;
}

bool Payment::isPaymentCompleted() const {
    return isCompleted;
}

std::string Payment::getAssociatedTicketNumber() const {
    return ticket ? ticket->getTicketNumber() : "";
}
