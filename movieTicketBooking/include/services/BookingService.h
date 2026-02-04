#pragma once

#include <memory>
#include <vector>
#include <future>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <atomic>

#include "../models/Booking.h"
#include "../models/Show.h"
#include "../repositories/BookingRepository.h"
#include "../repositories/ShowRepository.h"

namespace MovieBooking {
namespace Services {

// Booking result structure
struct BookingResult {
    bool success;
    std::string message;
    std::unique_ptr<Models::Booking> booking;
    std::vector<int> failedSeatIds;
    
    BookingResult(bool success, const std::string& message = "")
        : success(success), message(message) {}
};

// Seat selection request
struct SeatSelectionRequest {
    int showId;
    std::vector<int> seatIds;
    int userId;
    int lockDurationMinutes = 15;
    
    SeatSelectionRequest(int showId, const std::vector<int>& seatIds, int userId)
        : showId(showId), seatIds(seatIds), userId(userId) {}
};

// Booking service with thread safety and concurrency control
class BookingService {
private:
    std::unique_ptr<Repositories::BookingRepository> bookingRepository_;
    std::unique_ptr<Repositories::ShowRepository> showRepository_;
    
    // Thread safety for seat locking
    std::unordered_map<int, std::unique_ptr<std::mutex>> showMutexes_;
    std::mutex showMutexesMutex_;
    
    // Background cleanup thread
    std::atomic<bool> running_;
    std::thread cleanupThread_;
    std::condition_variable cleanupCondition_;
    std::mutex cleanupMutex_;
    
    // Configuration
    int defaultLockDurationMinutes_;
    int cleanupIntervalMinutes_;
    int maxBookingRetries_;
    
public:
    BookingService(std::unique_ptr<Repositories::BookingRepository> bookingRepository,
                   std::unique_ptr<Repositories::ShowRepository> showRepository,
                   int defaultLockDurationMinutes = 15,
                   int cleanupIntervalMinutes = 5,
                   int maxBookingRetries = 3);
    
    ~BookingService();

    // Core booking operations
    std::future<BookingResult> initiateBookingAsync(const SeatSelectionRequest& request);
    BookingResult initiateBooking(const SeatSelectionRequest& request);
    
    std::future<BookingResult> confirmBookingAsync(int bookingId, const std::string& paymentId);
    BookingResult confirmBooking(int bookingId, const std::string& paymentId);
    
    std::future<bool> cancelBookingAsync(int bookingId, int userId);
    bool cancelBooking(int bookingId, int userId);
    
    std::future<bool> releaseExpiredBookingAsync(int bookingId);
    bool releaseExpiredBooking(int bookingId);
    
    // Seat availability and selection
    std::future<std::vector<Models::ShowSeat*>> getAvailableSeatsAsync(int showId);
    std::vector<Models::ShowSeat*> getAvailableSeats(int showId);
    
    std::future<std::vector<int>> lockSeatsAsync(int showId, const std::vector<int>& seatIds, int bookingId);
    std::vector<int> lockSeats(int showId, const std::vector<int>& seatIds, int bookingId);
    
    // Booking queries
    std::future<std::vector<std::unique_ptr<Models::Booking>>> getUserBookingsAsync(int userId);
    std::vector<std::unique_ptr<Models::Booking>> getUserBookings(int userId);
    
    std::future<std::unique_ptr<Models::Booking>> getBookingAsync(int bookingId);
    std::unique_ptr<Models::Booking> getBooking(int bookingId);
    
    // Booking statistics and analytics
    std::future<int> getBookingCountAsync(int userId);
    int getBookingCount(int userId);
    
    std::future<double> getTotalSpentAsync(int userId);
    double getTotalSpent(int userId);
    
    // Batch operations
    std::future<bool> cancelExpiredBookingsAsync();
    bool cancelExpiredBookings();
    
    std::future<bool> releaseExpiredLocksAsync();
    bool releaseExpiredLocks();
    
    // Configuration
    void setDefaultLockDuration(int minutes) { defaultLockDurationMinutes_ = minutes; }
    void setCleanupInterval(int minutes) { cleanupIntervalMinutes_ = minutes; }
    void setMaxBookingRetries(int retries) { maxBookingRetries_ = retries; }
    
    // Service lifecycle
    void start();
    void stop();
    bool isRunning() const { return running_.load(); }

private:
    // Thread safety helpers
    std::mutex& getShowMutex(int showId);
    void cleanupShowMutex(int showId);
    
    // Core booking logic
    BookingResult processBookingRequest(const SeatSelectionRequest& request);
    bool validateSeatSelection(const SeatSelectionRequest& request);
    double calculateTotalPrice(const std::vector<Models::ShowSeat*>& seats);
    std::unique_ptr<Models::Booking> createPendingBooking(const SeatSelectionRequest& request, double totalPrice);
    
    // Seat management
    bool attemptSeatLocking(int showId, const std::vector<int>& seatIds, int bookingId);
    void releaseSeatLocks(int showId, const std::vector<int>& seatIds, int bookingId);
    bool confirmSeatBooking(int showId, const std::vector<int>& seatIds, int bookingId);
    
    // Background cleanup
    void cleanupWorker();
    void processExpiredBookings();
    void processExpiredLocks();
    
    // Error handling and logging
    void logBookingAttempt(const SeatSelectionRequest& request, const BookingResult& result);
    void logError(const std::string& operation, const std::string& error);
    
    // Utility methods
    bool isUserAuthorized(int bookingId, int userId);
    std::chrono::system_clock::time_point calculateExpiryTime(int durationMinutes);
    std::string generateBookingReference();
};

// Booking manager for high-level operations
class BookingManager {
private:
    std::unique_ptr<BookingService> bookingService_;
    
public:
    explicit BookingManager(std::unique_ptr<BookingService> bookingService);
    
    // High-level booking workflow
    std::future<BookingResult> bookTicketsAsync(int userId, int showId, const std::vector<int>& seatIds);
    BookingResult bookTickets(int userId, int showId, const std::vector<int>& seatIds);
    
    // Payment workflow
    std::future<BookingResult> processPaymentAsync(int bookingId, const std::string& paymentMethod);
    BookingResult processPayment(int bookingId, const std::string& paymentMethod);
    
    // User operations
    std::future<std::vector<std::unique_ptr<Models::Booking>>> getUserBookingHistoryAsync(int userId);
    std::vector<std::unique_ptr<Models::Booking>> getUserBookingHistory(int userId);
    
    // Service management
    void startService() { bookingService_->start(); }
    void stopService() { bookingService_->stop(); }
};

} // namespace Services
} // namespace MovieBooking
