#pragma once

#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace MovieBooking {
namespace Models {

enum class SeatType {
    REGULAR,
    PREMIUM,
    RECLINER,
    VIP
};

class Seat {
private:
    int id_;
    int screenId_;
    std::string rowNumber_;
    int seatNumber_;
    SeatType seatType_;
    double priceMultiplier_;
    bool isAvailable_;

public:
    Seat(int id, int screenId, const std::string& rowNumber, int seatNumber,
         SeatType seatType = SeatType::REGULAR, double priceMultiplier = 1.0,
         bool isAvailable = true);

    // Getters
    int getId() const { return id_; }
    int getScreenId() const { return screenId_; }
    const std::string& getRowNumber() const { return rowNumber_; }
    int getSeatNumber() const { return seatNumber_; }
    SeatType getSeatType() const { return seatType_; }
    double getPriceMultiplier() const { return priceMultiplier_; }
    bool isAvailable() const { return isAvailable_; }

    // Setters
    void setAvailable(bool available) { isAvailable_ = available; }
    void setPriceMultiplier(double multiplier) { priceMultiplier_ = multiplier; }

    // Utility methods
    std::string getSeatTypeString() const;
    std::string getFullSeatNumber() const { return rowNumber_ + std::to_string(seatNumber_); }
    double calculatePrice(double basePrice) const { return basePrice * priceMultiplier_; }

    // Serialization
    std::string toJson() const;
};

class Screen {
private:
    int id_;
    int theaterId_;
    std::string name_;
    int totalSeats_;
    int rows_;
    int seatsPerRow_;
    std::vector<SeatType> seatTypes_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;

public:
    Screen(int id, int theaterId, const std::string& name, int totalSeats,
           int rows, int seatsPerRow, const std::vector<SeatType>& seatTypes);

    // Getters
    int getId() const { return id_; }
    int getTheaterId() const { return theaterId_; }
    const std::string& getName() const { return name_; }
    int getTotalSeats() const { return totalSeats_; }
    int getRows() const { return rows_; }
    int getSeatsPerRow() const { return seatsPerRow_; }
    const std::vector<SeatType>& getSeatTypes() const { return seatTypes_; }

    // Utility methods
    bool isValidSeatPosition(const std::string& rowNumber, int seatNumber) const;
    std::vector<std::string> getSeatTypeStrings() const;
    int getSeatCountByType(SeatType type) const;

    // Serialization
    std::string toJson() const;
    
    // Factory methods
    static std::unique_ptr<Screen> createFromDbRow(const std::vector<std::string>& row);
};

} // namespace Models
} // namespace MovieBooking
