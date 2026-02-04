#pragma once

#include "Repository.h"
#include "../models/Booking.h"

namespace MovieBooking {
namespace Repositories {

class BookingRepository : public Repository<Models::Booking> {
public:
    explicit BookingRepository(std::unique_ptr<DatabaseConnection> connection);
    
    // Custom queries for bookings
    std::vector<std::unique_ptr<Models::Booking>> findByUserId(int userId);
    std::vector<std::unique_ptr<Models::Booking>> findByShowId(int showId);
    std::vector<std::unique_ptr<Models::Booking>> findByStatus(Models::BookingStatus status);
    std::vector<std::unique_ptr<Models::Booking>> findByUserIdAndStatus(int userId, Models::BookingStatus status);
    std::vector<std::unique_ptr<Models::Booking>> findExpiredBookings();
    std::vector<std::unique_ptr<Models::Booking>> findPendingBookingsOlderThan(int minutes);
    
    // Booking statistics
    int countByUserId(int userId);
    int countByShowId(int showId);
    int countByStatus(Models::BookingStatus status);
    double totalRevenueByShowId(int showId);
    double totalRevenueByUserId(int userId);
    double totalRevenueByDateRange(const std::string& startDate, const std::string& endDate);
    
    // Batch operations
    bool updateStatusBatch(const std::vector<int>& bookingIds, Models::BookingStatus status);
    bool updatePaymentStatusBatch(const std::vector<int>& bookingIds, Models::PaymentStatus status);
    bool cancelExpiredBookings();
    
    // Concurrent booking operations
    bool lockSeatsForBooking(int bookingId, const std::vector<int>& showSeatIds);
    bool releaseSeatsForBooking(int bookingId);
    bool confirmSeatsForBooking(int bookingId);
    
    // Payment operations
    bool updatePaymentId(int bookingId, const std::string& paymentId);
    std::vector<std::unique_ptr<Models::Booking>> findBookingsWithFailedPayments();

protected:
    std::string getSelectColumns() const override;
    std::string getInsertColumns() const override;
    std::string getInsertValues(const Models::Booking& entity) const override;
    std::string getUpdateSetClause(const Models::Booking& entity) const override;

private:
    // Helper methods for complex queries
    std::string buildBookingSeatsJoinQuery() const;
    std::string buildShowSeatsJoinQuery() const;
    std::string buildUserJoinQuery() const;
    std::string buildShowJoinQuery() const;
    
    // Row mappers for complex queries
    std::unique_ptr<Models::Booking> mapBookingWithSeats(const std::vector<std::string>& row);
    std::unique_ptr<Models::Booking> mapBookingWithDetails(const std::vector<std::string>& row);
};

} // namespace Repositories
} // namespace MovieBooking
