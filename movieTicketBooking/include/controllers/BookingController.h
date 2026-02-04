#pragma once

#include <memory>
#include <string>
#include <vector>
#include <future>
#include <functional>

#include "../services/BookingService.h"
#include "../payment/PaymentGateway.h"

// HTTP response structure
struct HttpResponse {
    int statusCode;
    std::string contentType;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
    
    HttpResponse(int code = 200, const std::string& type = "application/json")
        : statusCode(code), contentType(type) {}
};

// HTTP request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::unordered_map<std::string, std::string> headers;
    std::unordered_map<std::string, std::string> queryParams;
    std::string body;
    std::unordered_map<std::string, std::string> pathParams;
    
    std::string getHeader(const std::string& name) const {
        auto it = headers.find(name);
        return it != headers.end() ? it->second : "";
    }
    
    std::string getQueryParam(const std::string& name) const {
        auto it = queryParams.find(name);
        return it != queryParams.end() ? it->second : "";
    }
    
    std::string getPathParam(const std::string& name) const {
        auto it = pathParams.find(name);
        return it != pathParams.end() ? it->second : "";
    }
};

namespace MovieBooking {
namespace Controllers {

// Booking controller for handling HTTP requests
class BookingController {
private:
    std::unique_ptr<Services::BookingService> bookingService_;
    std::unique_ptr<Payment::PaymentService> paymentService_;
    
    // Request validation
    std::function<bool(const HttpRequest&)> authValidator_;
    std::function<bool(const HttpRequest&)> rateLimiter_;
    
    // Error handling
    std::function<HttpResponse(const std::string&, int)> errorHandler_;
    
public:
    BookingController(std::unique_ptr<Services::BookingService> bookingService,
                     std::unique_ptr<Payment::PaymentService> paymentService);
    
    // Core booking endpoints
    std::future<HttpResponse> initiateBookingAsync(const HttpRequest& request);
    HttpResponse initiateBooking(const HttpRequest& request);
    
    std::future<HttpResponse> confirmBookingAsync(const HttpRequest& request);
    HttpResponse confirmBooking(const HttpRequest& request);
    
    std::future<HttpResponse> cancelBookingAsync(const HttpRequest& request);
    HttpResponse cancelBooking(const HttpRequest& request);
    
    std::future<HttpResponse> getBookingAsync(const HttpRequest& request);
    HttpResponse getBooking(const HttpRequest& request);
    
    std::future<HttpResponse> getUserBookingsAsync(const HttpRequest& request);
    HttpResponse getUserBookings(const HttpRequest& request);
    
    // Seat selection endpoints
    std::future<HttpResponse> getAvailableSeatsAsync(const HttpRequest& request);
    HttpResponse getAvailableSeats(const HttpRequest& request);
    
    std::future<HttpResponse> lockSeatsAsync(const HttpRequest& request);
    HttpResponse lockSeats(const HttpRequest& request);
    
    // Payment endpoints
    std::future<HttpResponse> processPaymentAsync(const HttpRequest& request);
    HttpResponse processPayment(const HttpRequest& request);
    
    std::future<HttpResponse> getPaymentStatusAsync(const HttpRequest& request);
    HttpResponse getPaymentStatus(const HttpRequest& request);
    
    // Statistics and analytics endpoints
    std::future<HttpResponse> getBookingStatsAsync(const HttpRequest& request);
    HttpResponse getBookingStats(const HttpRequest& request);
    
    std::future<HttpResponse> getUserStatsAsync(const HttpRequest& request);
    HttpResponse getUserStats(const HttpRequest& request);
    
    // Configuration
    void setAuthValidator(std::function<bool(const HttpRequest&)> validator) {
        authValidator_ = validator;
    }
    
    void setRateLimiter(std::function<bool(const HttpRequest&)> limiter) {
        rateLimiter_ = limiter;
    }
    
    void setErrorHandler(std::function<HttpResponse(const std::string&, int)> handler) {
        errorHandler_ = handler;
    }

private:
    // Request parsing and validation
    Services::SeatSelectionRequest parseSeatSelectionRequest(const HttpRequest& request);
    Payment::PaymentRequest parsePaymentRequest(const HttpRequest& request);
    int extractUserId(const HttpRequest& request);
    int extractBookingId(const HttpRequest& request);
    
    // Response building
    HttpResponse buildSuccessResponse(const std::string& data);
    HttpResponse buildErrorResponse(const std::string& message, int statusCode = 400);
    HttpResponse buildBookingResponse(const Services::BookingResult& result);
    HttpResponse buildPaymentResponse(const Payment::PaymentResponse& response);
    
    // Validation helpers
    bool validateBookingRequest(const HttpRequest& request);
    bool validatePaymentRequest(const HttpRequest& request);
    bool validateUserPermissions(int userId, const HttpRequest& request);
    
    // Error handling
    HttpResponse handleException(const std::exception& e);
    HttpResponse handleValidationError(const std::string& field, const std::string& message);
};

// Show controller
class ShowController {
private:
    std::unique_ptr<Services::ShowService> showService_;
    std::function<bool(const HttpRequest&)> authValidator_;
    
public:
    explicit ShowController(std::unique_ptr<Services::ShowService> showService);
    
    // Show CRUD endpoints
    std::future<HttpResponse> createShowAsync(const HttpRequest& request);
    HttpResponse createShow(const HttpRequest& request);
    
    std::future<HttpResponse> getShowAsync(const HttpRequest& request);
    HttpResponse getShow(const HttpRequest& request);
    
    std::future<HttpResponse> updateShowAsync(const HttpRequest& request);
    HttpResponse updateShow(const HttpRequest& request);
    
    std::future<HttpResponse> cancelShowAsync(const HttpRequest& request);
    HttpResponse cancelShow(const HttpRequest& request);
    
    // Show search endpoints
    std::future<HttpResponse> searchShowsAsync(const HttpRequest& request);
    HttpResponse searchShows(const HttpRequest& request);
    
    std::future<HttpResponse> getShowsByMovieAsync(const HttpRequest& request);
    HttpResponse getShowsByMovie(const HttpRequest& request);
    
    std::future<HttpResponse> getUpcomingShowsAsync(const HttpRequest& request);
    HttpResponse getUpcomingShows(const HttpRequest& request);
    
    // Seat layout endpoints
    std::future<HttpResponse> getSeatingLayoutAsync(const HttpRequest& request);
    HttpResponse getSeatingLayout(const HttpRequest& request);
    
    std::future<HttpResponse> getAvailableSeatsAsync(const HttpRequest& request);
    HttpResponse getAvailableSeats(const HttpRequest& request);
    
    // Configuration
    void setAuthValidator(std::function<bool(const HttpRequest&)> validator) {
        authValidator_ = validator;
    }

private:
    Services::ShowCreationRequest parseShowCreationRequest(const HttpRequest& request);
    Services::ShowSearchCriteria parseShowSearchCriteria(const HttpRequest& request);
    HttpResponse buildShowResponse(const std::unique_ptr<Models::Show>& show);
    HttpResponse buildShowsResponse(const std::vector<std::unique_ptr<Models::Show>>& shows);
};

// Router for handling HTTP routing
class Router {
private:
    std::unordered_map<std::string, std::unordered_map<std::string, std::function<HttpResponse(const HttpRequest&)>>> routes_;
    std::unordered_map<std::string, std::unique_ptr<BookingController>> bookingControllers_;
    std::unordered_map<std::string, std::unique_ptr<ShowController>> showControllers_;
    
public:
    // Route registration
    void registerRoute(const std::string& method, const std::string& path, 
                      std::function<HttpResponse(const HttpRequest&)> handler);
    
    void registerBookingController(const std::string& prefix, std::unique_ptr<BookingController> controller);
    void registerShowController(const std::string& prefix, std::unique_ptr<ShowController> controller);
    
    // Request handling
    HttpResponse handleRequest(const HttpRequest& request);
    
    // Route configuration
    void setupDefaultRoutes();
    void setupBookingRoutes();
    void setupShowRoutes();

private:
    std::pair<std::string, std::unordered_map<std::string, std::string>> parsePath(const std::string& path);
    std::function<HttpResponse(const HttpRequest&)> findHandler(const std::string& method, const std::string& path);
};

} // namespace Controllers
} // namespace MovieBooking
