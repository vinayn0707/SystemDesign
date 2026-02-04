#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>

namespace MovieBooking {
namespace Models {

enum class BookingStatus {
    PENDING,
    CONFIRMED,
    CANCELLED,
    EXPIRED
};

enum class PaymentStatus {
    PENDING,
    PROCESSING,
    COMPLETED,
    FAILED,
    REFUNDED
};

class BookingSeat {
private:
    int id_;
    int bookingId_;
    int showSeatId_;
    double price_;

public:
    BookingSeat(int id, int bookingId, int showSeatId, double price);

    // Getters
    int getId() const { return id_; }
    int getBookingId() const { return bookingId_; }
    int getShowSeatId() const { return showSeatId_; }
    double getPrice() const { return price_; }

    // Serialization
    std::string toJson() const;
};

class Booking {
private:
    int id_;
    int userId_;
    int showId_;
    std::atomic<BookingStatus> bookingStatus_;
    std::atomic<PaymentStatus> paymentStatus_;
    double totalAmount_;
    std::chrono::system_clock::time_point bookingTime_;
    std::chrono::system_clock::time_point expiresAt_;
    std::vector<std::unique_ptr<BookingSeat>> bookingSeats_;
    std::string paymentId_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
    mutable std::mutex mutex_;

public:
    Booking(int id, int userId, int showId, double totalAmount,
            const std::chrono::system_clock::time_point& expiresAt,
            BookingStatus bookingStatus = BookingStatus::PENDING,
            PaymentStatus paymentStatus = PaymentStatus::PENDING);

    // Getters
    int getId() const { return id_; }
    int getUserId() const { return userId_; }
    int getShowId() const { return showId_; }
    BookingStatus getBookingStatus() const { return bookingStatus_.load(); }
    PaymentStatus getPaymentStatus() const { return paymentStatus_.load(); }
    double getTotalAmount() const { return totalAmount_; }
    std::chrono::system_clock::time_point getBookingTime() const { return bookingTime_; }
    std::chrono::system_clock::time_point getExpiresAt() const { return expiresAt_; }
    const std::string& getPaymentId() const { return paymentId_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Seat management
    void addBookingSeat(std::unique_ptr<BookingSeat> bookingSeat);
    const std::vector<std::unique_ptr<BookingSeat>>& getBookingSeats() const { return bookingSeats_; }
    std::vector<int> getShowSeatIds() const;

    // Thread-safe operations
    bool isExpired() const;
    bool canBeConfirmed() const;
    bool canBeCancelled() const;
    bool confirmBooking();
    bool cancelBooking();
    bool expireBooking();
    bool updatePaymentStatus(PaymentStatus status);
    bool setPaymentId(const std::string& paymentId);

    // Utility methods
    std::string getBookingStatusString() const;
    std::string getPaymentStatusString() const;
    std::string getBookingTimeString() const;
    std::string getExpiresAtString() const;
    int getSeatCount() const { return static_cast<int>(bookingSeats_.size()); }

    // Serialization
    std::string toJson() const;
    
    // Factory methods
    static std::unique_ptr<Booking> createFromDbRow(const std::vector<std::string>& row);
    static std::unique_ptr<Booking> createPending(int userId, int showId, 
                                                  const std::vector<int>& showSeatIds,
                                                  double totalAmount, int lockDurationMinutes = 15);
};

} // namespace Models
} // namespace MovieBooking
