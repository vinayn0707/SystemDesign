#pragma once

#include <string>
#include <exception>
#include <stdexcept>

namespace MovieBooking {
namespace Utils {

// Base exception class for the movie booking system
class MovieBookingException : public std::exception {
private:
    std::string message_;
    std::string errorCode_;
    int httpStatusCode_;

public:
    MovieBookingException(const std::string& message, const std::string& errorCode = "", 
                         int httpStatusCode = 500)
        : message_(message), errorCode_(errorCode), httpStatusCode_(httpStatusCode) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    const std::string& getErrorCode() const { return errorCode_; }
    int getHttpStatusCode() const { return httpStatusCode_; }
    
    virtual std::string toJson() const;
};

// Validation exceptions
class ValidationException : public MovieBookingException {
public:
    ValidationException(const std::string& message, const std::string& field = "")
        : MovieBookingException(message, "VALIDATION_ERROR", 400), field_(field) {}
    
    const std::string& getField() const { return field_; }
    
private:
    std::string field_;
};

// Authentication and authorization exceptions
class AuthenticationException : public MovieBookingException {
public:
    AuthenticationException(const std::string& message = "Authentication failed")
        : MovieBookingException(message, "AUTHENTICATION_ERROR", 401) {}
};

class AuthorizationException : public MovieBookingException {
public:
    AuthorizationException(const std::string& message = "Access denied")
        : MovieBookingException(message, "AUTHORIZATION_ERROR", 403) {}
};

// Resource not found exceptions
class ResourceNotFoundException : public MovieBookingException {
private:
    std::string resourceType_;
    std::string resourceId_;
    
public:
    ResourceNotFoundException(const std::string& resourceType, const std::string& resourceId)
        : MovieBookingException(resourceType + " with ID '" + resourceId + "' not found", 
                               "RESOURCE_NOT_FOUND", 404),
          resourceType_(resourceType), resourceId_(resourceId) {}
    
    const std::string& getResourceType() const { return resourceType_; }
    const std::string& getResourceId() const { return resourceId_; }
};

// Conflict exceptions
class ConflictException : public MovieBookingException {
public:
    ConflictException(const std::string& message, const std::string& conflictType = "")
        : MovieBookingException(message, "CONFLICT_ERROR", 409), conflictType_(conflictType) {}
    
    const std::string& getConflictType() const { return conflictType_; }
};

class SeatAlreadyBookedException : public ConflictException {
public:
    SeatAlreadyBookedException(int seatId, int showId)
        : ConflictException("Seat " + std::to_string(seatId) + " is already booked for show " + std::to_string(showId),
                           "SEAT_ALREADY_BOOKED") {}
};

class ShowTimeConflictException : public ConflictException {
public:
    ShowTimeConflictException(int screenId)
        : ConflictException("Show time conflict detected for screen " + std::to_string(screenId),
                           "SHOW_TIME_CONFLICT") {}
};

// Business logic exceptions
class BusinessRuleException : public MovieBookingException {
public:
    BusinessRuleException(const std::string& message, const std::string& rule = "")
        : MovieBookingException(message, "BUSINESS_RULE_VIOLATION", 422), rule_(rule) {}
    
    const std::string& getRule() const { return rule_; }
};

class BookingExpiredException : public BusinessRuleException {
public:
    BookingExpiredException(int bookingId)
        : BusinessRuleException("Booking " + std::to_string(bookingId) + " has expired",
                               "BOOKING_EXPIRED") {}
};

class InsufficientSeatsException : public BusinessRuleException {
public:
    InsufficientSeatsException(int requested, int available)
        : BusinessRuleException("Insufficient seats available. Requested: " + std::to_string(requested) + 
                               ", Available: " + std::to_string(available),
                               "INSUFFICIENT_SEATS") {}
};

class PaymentException : public MovieBookingException {
private:
    std::string paymentId_;
    std::string gateway_;
    
public:
    PaymentException(const std::string& message, const std::string& paymentId = "", 
                    const std::string& gateway = "")
        : MovieBookingException(message, "PAYMENT_ERROR", 402),
          paymentId_(paymentId), gateway_(gateway) {}
    
    const std::string& getPaymentId() const { return paymentId_; }
    const std::string& getGateway() const { return gateway_; }
};

class PaymentFailedException : public PaymentException {
public:
    PaymentFailedException(const std::string& paymentId, const std::string& reason = "")
        : PaymentException("Payment failed: " + reason, paymentId) {}
};

class RefundException : public PaymentException {
public:
    RefundException(const std::string& paymentId, const std::string& reason = "")
        : PaymentException("Refund failed: " + reason, paymentId) {}
};

// Database exceptions
class DatabaseException : public MovieBookingException {
private:
    std::string query_;
    std::string databaseError_;
    
public:
    DatabaseException(const std::string& message, const std::string& query = "", 
                     const std::string& databaseError = "")
        : MovieBookingException(message, "DATABASE_ERROR", 500),
          query_(query), databaseError_(databaseError) {}
    
    const std::string& getQuery() const { return query_; }
    const std::string& getDatabaseError() const { return databaseError_; }
};

class ConnectionException : public DatabaseException {
public:
    ConnectionException(const std::string& message)
        : DatabaseException("Database connection failed: " + message) {}
};

class QueryException : public DatabaseException {
public:
    QueryException(const std::string& message, const std::string& query = "")
        : DatabaseException("Query execution failed: " + message, query) {}
};

// Concurrency exceptions
class ConcurrencyException : public MovieBookingException {
public:
    ConcurrencyException(const std::string& message)
        : MovieBookingException(message, "CONCURRENCY_ERROR", 409) {}
};

class LockTimeoutException : public ConcurrencyException {
public:
    LockTimeoutException(const std::string& resource, int timeoutMs)
        : ConcurrencyException("Failed to acquire lock on " + resource + 
                              " within " + std::to_string(timeoutMs) + "ms") {}
};

class DeadlockException : public ConcurrencyException {
public:
    DeadlockException(const std::string& message = "Deadlock detected")
        : ConcurrencyException(message) {}
};

// Rate limiting exceptions
class RateLimitException : public MovieBookingException {
private:
    int retryAfterSeconds_;
    
public:
    RateLimitException(const std::string& message, int retryAfterSeconds = 60)
        : MovieBookingException(message, "RATE_LIMIT_EXCEEDED", 429),
          retryAfterSeconds_(retryAfterSeconds) {}
    
    int getRetryAfterSeconds() const { return retryAfterSeconds_; }
};

// Configuration exceptions
class ConfigurationException : public MovieBookingException {
public:
    ConfigurationException(const std::string& message, const std::string& configKey = "")
        : MovieBookingException(message, "CONFIGURATION_ERROR", 500), configKey_(configKey) {}
    
    const std::string& getConfigKey() const { return configKey_; }
    
private:
    std::string configKey_;
};

// External service exceptions
class ExternalServiceException : public MovieBookingException {
private:
    std::string serviceName_;
    std::string serviceError_;
    
public:
    ExternalServiceException(const std::string& serviceName, const std::string& message,
                             const std::string& serviceError = "")
        : MovieBookingException(serviceName + " service error: " + message, 
                               "EXTERNAL_SERVICE_ERROR", 502),
          serviceName_(serviceName), serviceError_(serviceError) {}
    
    const std::string& getServiceName() const { return serviceName_; }
    const std::string& getServiceError() const { return serviceError_; }
};

class PaymentGatewayException : public ExternalServiceException {
public:
    PaymentGatewayException(const std::string& gateway, const std::string& message)
        : ExternalServiceException(gateway + " payment gateway", message) {}
};

// Utility functions for exception handling
namespace ExceptionUtils {

// Convert exception to JSON error response
std::string exceptionToJson(const std::exception& ex);

// Extract error code from exception
std::string getErrorCode(const std::exception& ex);

// Extract HTTP status code from exception
int getHttpStatusCode(const std::exception& ex);

// Check if exception is retryable
bool isRetryable(const std::exception& ex);

// Create appropriate exception from error code
std::unique_ptr<MovieBookingException> createExceptionFromErrorCode(
    const std::string& errorCode, const std::string& message);

// Exception handler for logging and reporting
class ExceptionHandler {
private:
    bool enableLogging_;
    bool enableMetrics_;
    
public:
    ExceptionHandler(bool enableLogging = true, bool enableMetrics = true);
    
    void handleException(const std::exception& ex, const std::string& context = "");
    void handleUnexpectedException(const std::string& context = "");
    
    void setLoggingEnabled(bool enabled) { enableLogging_ = enabled; }
    void setMetricsEnabled(bool enabled) { enableMetrics_ = enabled; }

private:
    void logException(const std::exception& ex, const std::string& context);
    void reportMetrics(const std::exception& ex, const std::string& context);
};

} // namespace ExceptionUtils

} // namespace Utils
} // namespace MovieBooking
