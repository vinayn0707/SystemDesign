#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <vector>

namespace MovieBooking {
namespace Models {

enum class MovieStatus {
    NOW_SHOWING,
    COMING_SOON,
    ENDED
};

class Movie {
private:
    int id_;
    std::string title_;
    std::string description_;
    int durationMinutes_;
    std::string genre_;
    std::string language_;
    std::string rating_;
    std::chrono::system_clock::time_point releaseDate_;
    MovieStatus status_;
    std::string posterUrl_;
    std::chrono::system_clock::time_point createdAt_;
    std::chrono::system_clock::time_point updatedAt_;

public:
    // Constructor
    Movie(int id, const std::string& title, const std::string& description,
         int durationMinutes, const std::string& genre, const std::string& language,
         const std::string& rating, const std::chrono::system_clock::time_point& releaseDate,
         MovieStatus status = MovieStatus::COMING_SOON,
         const std::string& posterUrl = "");

    // Getters
    int getId() const { return id_; }
    const std::string& getTitle() const { return title_; }
    const std::string& getDescription() const { return description_; }
    int getDurationMinutes() const { return durationMinutes_; }
    const std::string& getGenre() const { return genre_; }
    const std::string& getLanguage() const { return language_; }
    const std::string& getRating() const { return rating_; }
    std::chrono::system_clock::time_point getReleaseDate() const { return releaseDate_; }
    MovieStatus getStatus() const { return status_; }
    const std::string& getPosterUrl() const { return posterUrl_; }
    std::chrono::system_clock::time_point getCreatedAt() const { return createdAt_; }
    std::chrono::system_clock::time_point getUpdatedAt() const { return updatedAt_; }

    // Setters
    void setTitle(const std::string& title);
    void setDescription(const std::string& description);
    void setStatus(MovieStatus status);
    void setPosterUrl(const std::string& posterUrl);

    // Utility methods
    std::string getStatusString() const;
    std::string getReleaseDateString() const;
    bool isNowShowing() const { return status_ == MovieStatus::NOW_SHOWING; }
    bool isComingSoon() const { return status_ == MovieStatus::COMING_SOON; }
    bool hasEnded() const { return status_ == MovieStatus::ENDED; }

    // Serialization
    std::string toJson() const;
    
    // Factory methods
    static std::unique_ptr<Movie> createFromJson(const std::string& json);
    static std::unique_ptr<Movie> createFromDbRow(const std::vector<std::string>& row);
};

} // namespace Models
} // namespace MovieBooking
