#pragma once

#include <string>
#include <memory>
#include <vector>
#include <future>
#include <mutex>
#include <unordered_map>
#include <chrono>
#include <functional>

namespace MovieBooking {
namespace Payment {

// Payment method types
enum class PaymentMethod {
    CREDIT_CARD,
    DEBIT_CARD,
    UPI,
    NET_BANKING,
    WALLET
};

// Payment status
enum class PaymentStatus {
    PENDING,
    PROCESSING,
    COMPLETED,
    FAILED,
    REFUNDED,
    CANCELLED
};

// Payment request structure
struct PaymentRequest {
    std::string bookingId;
    double amount;
    PaymentMethod method;
    std::string currency;
    std::string description;
    std::unordered_map<std::string, std::string> paymentDetails;
    std::string returnUrl;
    std::string cancelUrl;
    std::chrono::system_clock::time_point timestamp;
    
    PaymentRequest(const std::string& bookingId, double amount, PaymentMethod method)
        : bookingId(bookingId), amount(amount), method(method), currency("USD"), 
          timestamp(std::chrono::system_clock::now()) {}
};

// Payment response structure
struct PaymentResponse {
    bool success;
    std::string transactionId;
    std::string paymentId;
    PaymentStatus status;
    std::string message;
    std::string gatewayResponse;
    std::chrono::system_clock::time_point processedAt;
    std::unordered_map<std::string, std::string> additionalData;
    
    PaymentResponse(bool success, const std::string& message = "")
        : success(success), status(PaymentStatus::FAILED), message(message),
          processedAt(std::chrono::system_clock::now()) {}
};

// Refund request structure
struct RefundRequest {
    std::string originalTransactionId;
    std::string bookingId;
    double amount;
    std::string reason;
    std::chrono::system_clock::time_point timestamp;
    
    RefundRequest(const std::string& originalTransactionId, const std::string& bookingId, double amount)
        : originalTransactionId(originalTransactionId), bookingId(bookingId), amount(amount),
          reason("Customer request"), timestamp(std::chrono::system_clock::now()) {}
};

// Refund response structure
struct RefundResponse {
    bool success;
    std::string refundId;
    std::string transactionId;
    double refundedAmount;
    std::string status;
    std::string message;
    std::chrono::system_clock::time_point processedAt;
    
    RefundResponse(bool success, const std::string& message = "")
        : success(success), refundedAmount(0.0), status("failed"), message(message),
          processedAt(std::chrono::system_clock::now()) {}
};

// Abstract payment gateway interface
class IPaymentGateway {
public:
    virtual ~IPaymentGateway() = default;
    
    // Core payment operations
    virtual std::future<PaymentResponse> processPaymentAsync(const PaymentRequest& request) = 0;
    virtual PaymentResponse processPayment(const PaymentRequest& request) = 0;
    
    // Refund operations
    virtual std::future<RefundResponse> processRefundAsync(const RefundRequest& request) = 0;
    virtual RefundResponse processRefund(const RefundRequest& request) = 0;
    
    // Payment status checking
    virtual std::future<PaymentStatus> checkPaymentStatusAsync(const std::string& transactionId) = 0;
    virtual PaymentStatus checkPaymentStatus(const std::string& transactionId) = 0;
    
    // Gateway information
    virtual std::string getGatewayName() const = 0;
    virtual std::vector<PaymentMethod> getSupportedMethods() const = 0;
    virtual bool isMethodSupported(PaymentMethod method) const = 0;
    
    // Validation
    virtual bool validatePaymentDetails(const PaymentRequest& request) = 0;
    virtual bool validateRefundDetails(const RefundRequest& request) = 0;
};

// Mock payment gateway for testing
class MockPaymentGateway : public IPaymentGateway {
private:
    std::unordered_map<std::string, PaymentResponse> transactions_;
    mutable std::mutex transactionsMutex_;
    double successRate_;
    std::chrono::milliseconds processingDelay_;
    
public:
    explicit MockPaymentGateway(double successRate = 0.95, std::chrono::milliseconds processingDelay = std::chrono::milliseconds(1000));
    
    std::future<PaymentResponse> processPaymentAsync(const PaymentRequest& request) override;
    PaymentResponse processPayment(const PaymentRequest& request) override;
    
    std::future<RefundResponse> processRefundAsync(const RefundRequest& request) override;
    RefundResponse processRefund(const RefundRequest& request) override;
    
    std::future<PaymentStatus> checkPaymentStatusAsync(const std::string& transactionId) override;
    PaymentStatus checkPaymentStatus(const std::string& transactionId) override;
    
    std::string getGatewayName() const override { return "MockGateway"; }
    std::vector<PaymentMethod> getSupportedMethods() const override;
    bool isMethodSupported(PaymentMethod method) const override;
    
    bool validatePaymentDetails(const PaymentRequest& request) override;
    bool validateRefundDetails(const RefundRequest& request) override;
    
    // Mock configuration
    void setSuccessRate(double rate) { successRate_ = rate; }
    void setProcessingDelay(std::chrono::milliseconds delay) { processingDelay_ = delay; }

private:
    std::string generateTransactionId();
    std::string generateRefundId();
    bool shouldSucceed();
};

// Stripe payment gateway implementation (conceptual)
class StripePaymentGateway : public IPaymentGateway {
private:
    std::string apiKey_;
    std::string webhookSecret_;
    std::string baseUrl_;
    
public:
    StripePaymentGateway(const std::string& apiKey, const std::string& webhookSecret = "",
                        const std::string& baseUrl = "https://api.stripe.com/v1");
    
    std::future<PaymentResponse> processPaymentAsync(const PaymentRequest& request) override;
    PaymentResponse processPayment(const PaymentRequest& request) override;
    
    std::future<RefundResponse> processRefundAsync(const RefundRequest& request) override;
    RefundResponse processRefund(const RefundRequest& request) override;
    
    std::future<PaymentStatus> checkPaymentStatusAsync(const std::string& transactionId) override;
    PaymentStatus checkPaymentStatus(const std::string& transactionId) override;
    
    std::string getGatewayName() const override { return "Stripe"; }
    std::vector<PaymentMethod> getSupportedMethods() const override;
    bool isMethodSupported(PaymentMethod method) const override;
    
    bool validatePaymentDetails(const PaymentRequest& request) override;
    bool validateRefundDetails(const RefundRequest& request) override;

private:
    std::string createStripePaymentIntent(const PaymentRequest& request);
    std::string confirmStripePayment(const std::string& paymentIntentId);
    std::string createStripeRefund(const RefundRequest& request);
    std::string makeHttpRequest(const std::string& endpoint, const std::string& method, 
                               const std::string& body = "");
    PaymentResponse parseStripeResponse(const std::string& response);
};

// Payment gateway factory
class PaymentGatewayFactory {
private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<IPaymentGateway>()>> gateways_;
    
public:
    static void registerGateway(const std::string& name, 
                               std::function<std::unique_ptr<IPaymentGateway>()> factory);
    
    static std::unique_ptr<IPaymentGateway> createGateway(const std::string& name);
    static std::vector<std::string> getAvailableGateways();
    
    // Convenience methods
    static std::unique_ptr<IPaymentGateway> createMockGateway(double successRate = 0.95);
    static std::unique_ptr<IPaymentGateway> createStripeGateway(const std::string& apiKey);
};

// Payment service for managing multiple gateways
class PaymentService {
private:
    std::unordered_map<std::string, std::unique_ptr<IPaymentGateway>> gateways_;
    std::string defaultGateway_;
    mutable std::mutex gatewaysMutex_;
    
    // Payment retry configuration
    int maxRetries_;
    std::chrono::seconds retryDelay_;
    
    // Transaction logging
    bool enableLogging_;
    std::function<void(const PaymentRequest&, const PaymentResponse&)> paymentLogger_;
    
public:
    PaymentService(const std::string& defaultGateway = "mock", int maxRetries = 3,
                  std::chrono::seconds retryDelay = std::chrono::seconds(2));
    
    // Gateway management
    void addGateway(const std::string& name, std::unique_ptr<IPaymentGateway> gateway);
    void setDefaultGateway(const std::string& name);
    std::string getDefaultGateway() const { return defaultGateway_; }
    
    // Payment operations with retry logic
    std::future<PaymentResponse> processPaymentAsync(const PaymentRequest& request, 
                                                     const std::string& gatewayName = "");
    PaymentResponse processPayment(const PaymentRequest& request, 
                                 const std::string& gatewayName = "");
    
    std::future<RefundResponse> processRefundAsync(const RefundRequest& request,
                                                   const std::string& gatewayName = "");
    RefundResponse processRefund(const RefundRequest& request,
                                const std::string& gatewayName = "");
    
    // Payment status
    std::future<PaymentStatus> checkPaymentStatusAsync(const std::string& transactionId,
                                                       const std::string& gatewayName = "");
    PaymentStatus checkPaymentStatus(const std::string& transactionId,
                                    const std::string& gatewayName = "");
    
    // Configuration
    void setMaxRetries(int retries) { maxRetries_ = retries; }
    void setRetryDelay(std::chrono::seconds delay) { retryDelay_ = delay; }
    void enableLogging(bool enable) { enableLogging_ = enable; }
    void setPaymentLogger(std::function<void(const PaymentRequest&, const PaymentResponse&)> logger);
    
    // Gateway information
    std::vector<std::string> getAvailableGateways() const;
    std::vector<PaymentMethod> getSupportedMethods(const std::string& gatewayName = "") const;

private:
    IPaymentGateway* getGateway(const std::string& name) const;
    PaymentResponse processPaymentWithRetry(const PaymentRequest& request, IPaymentGateway* gateway);
    RefundResponse processRefundWithRetry(const RefundRequest& request, IPaymentGateway* gateway);
    void logPayment(const PaymentRequest& request, const PaymentResponse& response);
};

} // namespace Payment
} // namespace MovieBooking
