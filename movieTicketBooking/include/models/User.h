#pragma once

#include <string>
#include <chrono>
#include <memory>

namespace MovieBooking {
namespace Models {

class User {
private:
    int id_;
    std::string username_;
    std::string email_;
    std::string passwordHash_;
    std::string firstName_;
    std::string lastName_;
    std::string phone_;
    bool isActive_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;

public:
    User(int id, const std::string& username, const std::string& email,
         const std::string& passwordHash, const std::string& firstName = "",
         const std::string& lastName = "", const std::string& phone = "",
         bool isActive = true);

    // Getters
    int getId() const { return id_; }
    const std::string& getUsername() const { return username_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getPasswordHash() const { return passwordHash_; }
    const std::string& getFirstName() const { return firstName_; }
    const std::string& getLastName() const { return lastName_; }
    const std::string& getPhone() const { return phone_; }
    bool isActive() const { return isActive_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setUsername(const std::string& username);
    void setEmail(const std::string& email);
    void setPasswordHash(const std::string& passwordHash);
    void setFirstName(const std::string& firstName);
    void setLastName(const std::string& lastName);
    void setPhone(const std::string& phone);
    void setActive(bool active);

    // Utility methods
    std::string getFullName() const;
    bool verifyPassword(const std::string& password) const;
    static std::string hashPassword(const std::string& password);

    // Serialization
    std::string toJson() const;
    
    // Factory methods
    static std::unique_ptr<User> createFromDbRow(const std::vector<std::string>& row);
    static std::unique_ptr<User> createNew(const std::string& username, const std::string& email,
                                           const std::string& password, const std::string& firstName = "",
                                           const std::string& lastName = "", const std::string& phone = "");
};

} // namespace Models
} // namespace MovieBooking
