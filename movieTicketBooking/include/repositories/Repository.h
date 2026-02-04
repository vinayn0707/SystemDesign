#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <future>

#include "DatabaseConnection.h"

namespace MovieBooking {
namespace Repositories {

// Base repository interface
template<typename T>
class IRepository {
public:
    virtual ~IRepository() = default;
    
    // CRUD operations
    virtual std::unique_ptr<T> findById(int id) = 0;
    virtual std::vector<std::unique_ptr<T>> findAll() = 0;
    virtual std::unique_ptr<T> save(const T& entity) = 0;
    virtual bool update(const T& entity) = 0;
    virtual bool deleteById(int id) = 0;
    
    // Custom queries
    virtual std::vector<std::unique_ptr<T>> findByCondition(const std::string& condition) = 0;
    virtual bool existsById(int id) = 0;
    virtual int count() = 0;
};

// Generic repository implementation
template<typename T>
class Repository : public IRepository<T> {
protected:
    std::unique_ptr<DatabaseConnection> connection_;
    std::string tableName_;
    std::function<std::unique_ptr<T>(const std::vector<std::string>&)> rowMapper_;
    std::function<std::string(const T&)> entitySerializer_;

public:
    Repository(std::unique_ptr<DatabaseConnection> connection,
               const std::string& tableName,
               std::function<std::unique_ptr<T>(const std::vector<std::string>&)> rowMapper,
               std::function<std::string(const T&)> entitySerializer);
    
    virtual ~Repository() = default;

    // CRUD operations
    std::unique_ptr<T> findById(int id) override;
    std::vector<std::unique_ptr<T>> findAll() override;
    std::unique_ptr<T> save(const T& entity) override;
    bool update(const T& entity) override;
    bool deleteById(int id) override;
    
    // Custom queries
    std::vector<std::unique_ptr<T>> findByCondition(const std::string& condition) override;
    bool existsById(int id) override;
    int count() override;
    
    // Batch operations
    std::vector<std::unique_ptr<T>> saveBatch(const std::vector<T>& entities);
    bool updateBatch(const std::vector<T>& entities);
    bool deleteBatch(const std::vector<int>& ids);
    
    // Async operations
    std::future<std::unique_ptr<T>> findByIdAsync(int id);
    std::future<std::vector<std::unique_ptr<T>>> findAllAsync();
    std::future<std::unique_ptr<T>> saveAsync(const T& entity);
    std::future<bool> updateAsync(const T& entity);
    std::future<bool> deleteByIdAsync(int id);

protected:
    // Helper methods
    std::string buildSelectQuery(const std::string& condition = "") const;
    std::string buildInsertQuery(const T& entity) const;
    std::string buildUpdateQuery(const T& entity) const;
    std::string buildDeleteQuery(int id) const;
    
    virtual std::string getSelectColumns() const = 0;
    virtual std::string getInsertColumns() const = 0;
    virtual std::string getInsertValues(const T& entity) const = 0;
    virtual std::string getUpdateSetClause(const T& entity) const = 0;
    virtual std::string getWhereClause(int id) const { return "id = " + std::to_string(id); }
};

// Transaction support
class Transaction {
private:
    std::unique_ptr<DatabaseConnection> connection_;
    bool isActive_;
    bool isCommitted_;

public:
    explicit Transaction(std::unique_ptr<DatabaseConnection> connection);
    ~Transaction();
    
    bool commit();
    bool rollback();
    bool isActive() const { return isActive_ && !isCommitted_; }
    
    // Get connection for operations within transaction
    DatabaseConnection* getConnection() const { return connection_.get(); }

private:
    void cleanup();
};

// Repository factory
template<typename T>
class RepositoryFactory {
public:
    static std::unique_ptr<Repository<T>> create(
        const std::string& tableName,
        std::function<std::unique_ptr<T>(const std::vector<std::string>&)> rowMapper,
        std::function<std::string(const T&)> entitySerializer);
};

} // namespace Repositories
} // namespace MovieBooking
