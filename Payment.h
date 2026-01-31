#ifndef PAYMENT_H
#define PAYMENT_H

#include "ParkingTicket.h"
#include <memory>

class Payment {
private:
    std::string paymentId;
    double amount;
    std::chrono::system_clock::time_point paymentTime;
    std::shared_ptr<ParkingTicket> ticket;
    bool isCompleted;

public:
    Payment(std::shared_ptr<ParkingTicket> ticket);
    
    // Core functionality
    bool processPayment(double amount);
    
    // Getters
    std::string getPaymentId() const;
    double getAmount() const;
    bool isPaymentCompleted() const;
    std::string getAssociatedTicketNumber() const;
};

#endif // PAYMENT_H
