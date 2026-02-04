#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <mysql/mysql.h>

namespace MovieBooking {
namespace Database {

class DatabaseConnection {
private:
    MYSQL* connection_;
    std::string host_;
    std::string username_;
    std::string password_;
    std::string database_;
    int port_;
    bool isConnected_;
    mutable std::mutex mutex_;

public:
    DatabaseConnection(const std::string& host, const std::string& username,
                      const std::string& password, const std::string& database,
                      int port = 3306);
    
    ~DatabaseConnection();

    // Connection management
    bool connect();
    void disconnect();
    bool isConnected() const { return isConnected_; }
    bool reconnect();
    
    // Query execution
    bool executeQuery(const std::string& query);
    std::vector<std::vector<std::string>> fetchResults(const std::string& query);
    std::vector<std::string> fetchRow(const std::string& query);
    std::string fetchSingleValue(const std::string& query);
    int getLastInsertId();
    
    // Transaction management
    bool beginTransaction();
    bool commit();
    bool rollback();
    
    // Prepared statements
    MYSQL_STMT* prepareStatement(const std::string& query);
    bool executePreparedStatement(MYSQL_STMT* stmt);
    void closeStatement(MYSQL_STMT* stmt);
    
    // Utility methods
    std::string escapeString(const std::string& input);
    bool ping();
    std::string getLastError() const;

private:
    void cleanup();
    bool checkConnection();
};

// Connection pool for better performance
class ConnectionPool {
private:
    std::vector<std::unique_ptr<DatabaseConnection>> connections_;
    std::string host_;
    std::string username_;
    std::string password_;
    std::string database_;
    int port_;
    int maxConnections_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;

public:
    ConnectionPool(const std::string& host, const std::string& username,
                  const std::string& password, const std::string& database,
                  int port = 3306, int maxConnections = 10);
    
    ~ConnectionPool();

    std::unique_ptr<DatabaseConnection> getConnection();
    void returnConnection(std::unique_ptr<DatabaseConnection> connection);
    
    int getAvailableConnections() const;
    int getTotalConnections() const { return maxConnections_; }

private:
    void initializePool();
};

// Singleton for database connection pool
class DatabaseManager {
private:
    static std::unique_ptr<ConnectionPool> pool_;
    static std::mutex mutex_;

public:
    static bool initialize(const std::string& host, const std::string& username,
                         const std::string& password, const std::string& database,
                         int port = 3306, int maxConnections = 10);
    
    static std::unique_ptr<DatabaseConnection> getConnection();
    static void shutdown();
    
    static bool isInitialized();
};

} // namespace Database
} // namespace MovieBooking
