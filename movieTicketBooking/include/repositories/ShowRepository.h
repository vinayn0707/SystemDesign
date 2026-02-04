#pragma once

#include "Repository.h"
#include "../models/Show.h"

namespace MovieBooking {
namespace Repositories {

class ShowRepository : public Repository<Models::Show> {
public:
    explicit ShowRepository(std::unique_ptr<DatabaseConnection> connection);
    
    // Custom queries for shows
    std::vector<std::unique_ptr<Models::Show>> findByMovieId(int movieId);
    std::vector<std::unique_ptr<Models::Show>> findByScreenId(int screenId);
    std::vector<std::unique_ptr<Models::Show>> findByStatus(Models::ShowStatus status);
    std::vector<std::unique_ptr<Models::Show>> findByDateRange(const std::string& startDate, const std::string& endDate);
    std::vector<std::unique_ptr<Models::Show>> findUpcomingShows(int days = 7);
    std::vector<std::unique_ptr<Models::Show>> findOngoingShows();
    std::vector<std::unique_ptr<Models::Show>> findShowsWithAvailableSeats();
    
    // Show seat operations
    bool createShowSeats(int showId, int screenId, double basePrice);
    std::vector<std::unique_ptr<Models::ShowSeat>> getShowSeats(int showId);
    std::vector<std::unique_ptr<Models::ShowSeat>> getAvailableShowSeats(int showId);
    std::vector<std::unique_ptr<Models::ShowSeat>> getLockedShowSeats(int showId);
    std::vector<std::unique_ptr<Models::ShowSeat>> getBookedShowSeats(int showId);
    
    // Seat locking operations (thread-safe)
    bool lockShowSeats(int showId, const std::vector<int>& seatIds, int bookingId, int lockDurationMinutes = 15);
    bool releaseShowSeats(int showId, const std::vector<int>& seatIds, int bookingId);
    bool bookShowSeats(int showId, const std::vector<int>& seatIds, int bookingId);
    bool releaseExpiredLocks(int showId);
    
    // Show statistics
    int getAvailableSeatCount(int showId);
    int getBookedSeatCount(int showId);
    int getLockedSeatCount(int showId);
    double calculateShowRevenue(int showId);
    double calculateOccupancyRate(int showId);
    
    // Batch operations
    bool updateShowStatusBatch(const std::vector<int>& showIds, Models::ShowStatus status);
    bool cancelShowsOlderThanDate(const std::string& date);
    
    // Conflict detection
    bool hasTimeConflict(int screenId, const std::chrono::system_clock::time_point& startTime,
                        const std::chrono::system_clock::time_point& endTime, int excludeShowId = -1);
    std::vector<std::unique_ptr<Models::Show>> findConflictingShows(int screenId,
                                                                   const std::chrono::system_clock::time_point& startTime,
                                                                   const std::chrono::system_clock::time_point& endTime);

protected:
    std::string getSelectColumns() const override;
    std::string getInsertColumns() const override;
    std::string getInsertValues(const Models::Show& entity) const override;
    std::string getUpdateSetClause(const Models::Show& entity) const override;

private:
    // Helper methods
    std::string buildShowSeatsQuery() const;
    std::string buildSeatLockQuery() const;
    std::string buildTimeConflictQuery() const;
    
    // Row mappers
    std::unique_ptr<Models::ShowSeat> mapShowSeat(const std::vector<std::string>& row);
    std::unique_ptr<Models::Show> mapShowWithSeats(const std::vector<std::string>& row);
    
    // Seat management helpers
    bool updateShowSeatStatus(int showSeatId, Models::ShowSeatStatus status, int bookingId = -1);
    bool updateShowSeatLock(int showSeatId, const std::chrono::system_clock::time_point& lockedUntil, int bookingId = -1);
};

} // namespace Repositories
} // namespace MovieBooking
