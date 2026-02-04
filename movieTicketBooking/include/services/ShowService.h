#pragma once

#include <memory>
#include <vector>
#include <future>
#include <mutex>
#include <unordered_map>
#include <chrono>

#include "../models/Show.h"
#include "../models/Movie.h"
#include "../models/Screen.h"
#include "../repositories/ShowRepository.h"

namespace MovieBooking {
namespace Services {

// Show creation request
struct ShowCreationRequest {
    int movieId;
    int screenId;
    std::chrono::system_clock::time_point startTime;
    std::chrono::system_clock::time_point endTime;
    double basePrice;
    
    ShowCreationRequest(int movieId, int screenId,
                       const std::chrono::system_clock::time_point& startTime,
                       const std::chrono::system_clock::time_point& endTime,
                       double basePrice)
        : movieId(movieId), screenId(screenId), startTime(startTime), endTime(endTime), basePrice(basePrice) {}
};

// Show search criteria
struct ShowSearchCriteria {
    std::vector<int> movieIds;
    std::vector<int> screenIds;
    std::vector<Models::ShowStatus> statuses;
    std::chrono::system_clock::time_point startDate;
    std::chrono::system_clock::time_point endDate;
    bool hasAvailableSeats = false;
    
    ShowSearchCriteria() {
        startDate = std::chrono::system_clock::time_point::min();
        endDate = std::chrono::system_clock::time_point::max();
    }
};

// Show service with caching and optimization
class ShowService {
private:
    std::unique_ptr<Repositories::ShowRepository> showRepository_;
    
    // Caching
    std::unordered_map<int, std::unique_ptr<Models::Show>> showCache_;
    std::mutex cacheMutex_;
    std::chrono::seconds cacheExpiry_;
    std::unordered_map<int, std::chrono::system_clock::time_point> cacheTimestamps_;
    
    // Configuration
    int maxCacheSize_;
    bool enableCaching_;
    
public:
    explicit ShowService(std::unique_ptr<Repositories::ShowRepository> showRepository,
                         int maxCacheSize = 1000, bool enableCaching = true);
    
    // Show CRUD operations
    std::future<std::unique_ptr<Models::Show>> createShowAsync(const ShowCreationRequest& request);
    std::unique_ptr<Models::Show> createShow(const ShowCreationRequest& request);
    
    std::future<std::unique_ptr<Models::Show>> getShowAsync(int showId);
    std::unique_ptr<Models::Show> getShow(int showId);
    
    std::future<bool> updateShowAsync(const Models::Show& show);
    bool updateShow(const Models::Show& show);
    
    std::future<bool> cancelShowAsync(int showId);
    bool cancelShow(int showId);
    
    // Show search and filtering
    std::future<std::vector<std::unique_ptr<Models::Show>>> searchShowsAsync(const ShowSearchCriteria& criteria);
    std::vector<std::unique_ptr<Models::Show>> searchShows(const ShowSearchCriteria& criteria);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> getShowsByMovieAsync(int movieId);
    std::vector<std::unique_ptr<Models::Show>> getShowsByMovie(int movieId);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> getShowsByScreenAsync(int screenId);
    std::vector<std::unique_ptr<Models::Show>> getShowsByScreen(int screenId);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> getUpcomingShowsAsync(int days = 7);
    std::vector<std::unique_ptr<Models::Show>> getUpcomingShows(int days = 7);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> getOngoingShowsAsync();
    std::vector<std::unique_ptr<Models::Show>> getOngoingShows();
    
    // Show seat operations
    std::future<std::vector<Models::ShowSeat*>> getAvailableSeatsAsync(int showId);
    std::vector<Models::ShowSeat*> getAvailableSeats(int showId);
    
    std::future<std::vector<Models::ShowSeat*>> getShowSeatingLayoutAsync(int showId);
    std::vector<Models::ShowSeat*> getShowSeatingLayout(int showId);
    
    std::future<std::vector<int>> lockSeatsAsync(int showId, const std::vector<int>& seatIds, int bookingId);
    std::vector<int> lockSeats(int showId, const std::vector<int>& seatIds, int bookingId);
    
    // Show statistics and analytics
    std::future<int> getAvailableSeatCountAsync(int showId);
    int getAvailableSeatCount(int showId);
    
    std::future<int> getBookedSeatCountAsync(int showId);
    int getBookedSeatCount(int showId);
    
    std::future<double> getOccupancyRateAsync(int showId);
    double getOccupancyRate(int showId);
    
    std::future<double> getExpectedRevenueAsync(int showId);
    double getExpectedRevenue(int showId);
    
    // Conflict detection and validation
    std::future<bool> hasTimeConflictAsync(const ShowCreationRequest& request, int excludeShowId = -1);
    bool hasTimeConflict(const ShowCreationRequest& request, int excludeShowId = -1);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> findConflictingShowsAsync(const ShowCreationRequest& request);
    std::vector<std::unique_ptr<Models::Show>> findConflictingShows(const ShowCreationRequest& request);
    
    // Batch operations
    std::future<bool> updateShowStatusBatchAsync(const std::vector<int>& showIds, Models::ShowStatus status);
    bool updateShowStatusBatch(const std::vector<int>& showIds, Models::ShowStatus status);
    
    std::future<bool> cancelOldShowsAsync(const std::chrono::system_clock::time_point& beforeDate);
    bool cancelOldShows(const std::chrono::system_clock::time_point& beforeDate);
    
    // Cache management
    void clearCache();
    void invalidateCache(int showId);
    void setCacheExpiry(std::chrono::seconds expiry) { cacheExpiry_ = expiry; }
    void setMaxCacheSize(int size) { maxCacheSize_ = size; }
    void enableCaching(bool enable) { enableCaching_ = enable; }

private:
    // Cache management
    void updateCache(int showId, std::unique_ptr<Models::Show> show);
    std::unique_ptr<Models::Show> getFromCache(int showId);
    bool isCacheValid(int showId);
    void cleanupExpiredCache();
    void evictOldestEntries();
    
    // Validation helpers
    bool validateShowRequest(const ShowCreationRequest& request);
    bool isValidTimeRange(const std::chrono::system_clock::time_point& startTime,
                         const std::chrono::system_clock::time_point& endTime);
    bool isValidScreen(int screenId);
    bool isValidMovie(int movieId);
    
    // Show creation helpers
    std::unique_ptr<Models::Show> buildShowFromRequest(const ShowCreationRequest& request);
    bool createShowSeats(int showId, int screenId, double basePrice);
    
    // Search helpers
    std::string buildSearchQuery(const ShowSearchCriteria& criteria);
    std::vector<std::unique_ptr<Models::Show>> executeSearchQuery(const std::string& query);
    
    // Statistics helpers
    double calculateOccupancyRate(int bookedSeats, int totalSeats);
    double calculateExpectedRevenue(int showId);
    
    // Error handling
    void logError(const std::string& operation, const std::string& error);
};

// Show manager for high-level operations
class ShowManager {
private:
    std::unique_ptr<ShowService> showService_;
    
public:
    explicit ShowManager(std::unique_ptr<ShowService> showService);
    
    // High-level show management
    std::future<std::unique_ptr<Models::Show>> scheduleShowAsync(const ShowCreationRequest& request);
    std::unique_ptr<Models::Show> scheduleShow(const ShowCreationRequest& request);
    
    // Show discovery
    std::future<std::vector<std::unique_ptr<Models::Show>>> findAvailableShowsAsync(
        int movieId, const std::chrono::system_clock::time_point& date);
    std::vector<std::unique_ptr<Models::Show>> findAvailableShows(
        int movieId, const std::chrono::system_clock::time_point& date);
    
    // Show analytics
    std::future<std::vector<std::unique_ptr<Models::Show>>> getPopularShowsAsync(int limit = 10);
    std::vector<std::unique_ptr<Models::Show>> getPopularShows(int limit = 10);
    
    std::future<std::vector<std::unique_ptr<Models::Show>>> getHighOccupancyShowsAsync(double threshold = 0.8);
    std::vector<std::unique_ptr<Models::Show>> getHighOccupancyShows(double threshold = 0.8);
};

} // namespace Services
} // namespace MovieBooking
