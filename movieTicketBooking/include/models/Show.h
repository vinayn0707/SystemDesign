#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include <mutex>
#include <atomic>

#include "Screen.h"

namespace MovieBooking {
namespace Models {

enum class ShowStatus {
    SCHEDULED,
    CANCELLED,
    COMPLETED,
    IN_PROGRESS
};

enum class ShowSeatStatus {
    AVAILABLE,
    LOCKED,
    BOOKED,
    MAINTENANCE
};

class ShowSeat {
private:
    int id_;
    int showId_;
    int seatId_;
    std::atomic<ShowSeatStatus> status_;
    std::chrono::system_clock::time_point lockedUntil_;
    int bookingId_;
    double price_;
    mutable std::mutex mutex_;

public:
    ShowSeat(int id, int showId, int seatId, ShowSeatStatus status = ShowSeatStatus::AVAILABLE,
             double price = 0.0);

    // Thread-safe getters
    int getId() const { return id_; }
    int getShowId() const { return showId_; }
    int getSeatId() const { return seatId_; }
    ShowSeatStatus getStatus() const { return status_.load(); }
    std::chrono::system_clock::time_point getLockedUntil() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return lockedUntil_; 
    }
    int getBookingId() const { 
        std::lock_guard<std::mutex> lock(mutex_);
        return bookingId_; 
    }
    double getPrice() const { return price_; }

    // Thread-safe operations
    bool isAvailable() const;
    bool canBeLocked(int lockDurationMinutes = 15) const;
    bool lockSeat(int bookingId, int lockDurationMinutes = 15);
    bool releaseLock();
    bool bookSeat(int bookingId);
    
    // Utility methods
    std::string getStatusString() const;
    std::string toJson() const;

    // Factory methods
    static std::unique_ptr<ShowSeat> createFromDbRow(const std::vector<std::string>& row);
};

class Show {
private:
    int id_;
    int movieId_;
    int screenId_;
    std::chrono::system_clock::time_point startTime_;
    std::chrono::system_clock::time_point endTime_;
    double basePrice_;
    std::atomic<ShowStatus> status_;
    std::vector<std::unique_ptr<ShowSeat>> showSeats_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;
    mutable std::shared_mutex mutex_;

public:
    Show(int id, int movieId, int screenId,
         const std::chrono::system_clock::time_point& startTime,
         const std::chrono::system_clock::time_point& endTime,
         double basePrice, ShowStatus status = ShowStatus::SCHEDULED);

    // Getters
    int getId() const { return id_; }
    int getMovieId() const { return movieId_; }
    int getScreenId() const { return screenId_; }
    std::chrono::system_clock::time_point getStartTime() const { return startTime_; }
    std::chrono::system_clock::time_point getEndTime() const { return endTime_; }
    double getBasePrice() const { return basePrice_; }
    ShowStatus getStatus() const { return status_.load(); }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setStatus(ShowStatus status) { status_.store(status); }

    // Seat management (thread-safe)
    void addShowSeat(std::unique_ptr<ShowSeat> showSeat);
    std::vector<ShowSeat*> getAvailableSeats() const;
    std::vector<ShowSeat*> getLockedSeats() const;
    std::vector<ShowSeat*> getBookedSeats() const;
    ShowSeat* getShowSeatById(int seatId) const;
    
    // Booking operations (thread-safe)
    bool lockSeats(const std::vector<int>& seatIds, int bookingId, int lockDurationMinutes = 15);
    bool releaseLockedSeats(int bookingId);
    bool bookSeats(const std::vector<int>& seatIds, int bookingId);
    
    // Utility methods
    std::string getStatusString() const;
    std::string getStartTimeString() const;
    std::string getEndTimeString() const;
    int getAvailableSeatCount() const;
    int getBookedSeatCount() const;
    double calculateTotalRevenue() const;

    // Serialization
    std::string toJson() const;
    
    // Factory methods
    static std::unique_ptr<Show> createFromDbRow(const std::vector<std::string>& row);
};

} // namespace Models
} // namespace MovieBooking
