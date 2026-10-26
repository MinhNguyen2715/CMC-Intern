#include "DatabaseManager.h"
#include "Utils.h"            
#include <iostream>

DatabaseManager::DatabaseManager() : m_pDB(nullptr) {}

DatabaseManager::~DatabaseManager() {
    Close();
}

void DatabaseManager::Close() {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (m_pDB) {
        sqlite3_close(m_pDB);
        m_pDB = nullptr;
    }
}

bool DatabaseManager::InitDatabase(const std::string& dbPath) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (sqlite3_open(dbPath.c_str(), &m_pDB) != SQLITE_OK) {
        return false;
    }

    // Write-Ahead Logging --> improves concurrency
    sqlite3_exec(m_pDB, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    
    const char* sqlUsers =
        "CREATE TABLE IF NOT EXISTS Users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password TEXT NOT NULL);";

    const char* sqlMessages =
        "CREATE TABLE IF NOT EXISTS Messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "sender TEXT NOT NULL, "
        "receiver TEXT NOT NULL, "
        "content TEXT NOT NULL, "
        "timestamp INTEGER DEFAULT (strftime('%s', 'now')));";

    const char* sqlIndexMessages =
        "CREATE INDEX IF NOT EXISTS idx_messages_conversation "
        "ON Messages(sender, receiver, timestamp);";

    char* errMsg = nullptr;
    if (sqlite3_exec(m_pDB, sqlUsers, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(m_pDB, sqlMessages, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(m_pDB, sqlIndexMessages, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool DatabaseManager::RegisterUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (!m_pDB) return false;
    // Prepared statement
    std::string sql = "INSERT INTO Users (username, password) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;

    
    if (sqlite3_prepare_v2(m_pDB, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // MD5 hash before storing
    std::string md5Password = Utils::GetMD5Hash(password);

    // Insert safe string to the prepared statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, md5Password.c_str(), -1, SQLITE_TRANSIENT);

    // Execute
    bool bSuccess = (sqlite3_step(stmt) == SQLITE_DONE);

    // Destroy prepared statement
    sqlite3_finalize(stmt);
    return bSuccess;
}

// Login authenticate
bool DatabaseManager::AuthenticateUser(const std::string& username, const std::string& password) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (!m_pDB) return false;

    std::string sql = "SELECT id FROM Users WHERE username = ? AND password = ?;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    // MD5 hash
    std::string md5Password = Utils::GetMD5Hash(password);

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, md5Password.c_str(), -1, SQLITE_TRANSIENT);

    bool bAuth = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return bAuth;
}

// Store messages
bool DatabaseManager::SaveMessage(const std::string& sender, const std::string& receiver, const std::string& content, int64_t timestamp) {
    std::lock_guard<std::mutex> lock(m_dbMutex);
    if (!m_pDB) return false;

    const char* sql = "INSERT INTO Messages (sender, receiver, content, timestamp) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, sender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, receiver.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, content.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 4, timestamp);
   
    bool bSuccess = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return bSuccess;
}

// Users list
std::vector<std::string> DatabaseManager::GetAllUsers(const std::string& currentUsername) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    std::vector<std::string> users;
    if (!m_pDB) return users;

    std::string sql = "SELECT username FROM Users WHERE username != ? ORDER BY username ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, currentUsername.c_str(), -1, SQLITE_TRANSIENT);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char* text = sqlite3_column_text(stmt, 0);
            if (text) {
                users.push_back(reinterpret_cast<const char*>(text));
            }
        }
        sqlite3_finalize(stmt);
    }
    return users;
}

// Chat history
std::vector<ChatMessage> DatabaseManager::GetChatHistory(const std::string& user1, const std::string& user2) {
    std::lock_guard<std::mutex> lock(m_dbMutex);

    std::vector<ChatMessage> history;
    if (!m_pDB) return history;

    std::string sql = "SELECT sender, receiver, content, timestamp FROM Messages "
        "WHERE (sender = ? AND receiver = ?) OR (sender = ? AND receiver = ?) "
        "ORDER BY timestamp ASC;";
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, user1.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, user2.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, user2.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, user1.c_str(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ChatMessage msg;
            const unsigned char* sText = sqlite3_column_text(stmt, 0);
            const unsigned char* rText = sqlite3_column_text(stmt, 1);
            const unsigned char* cText = sqlite3_column_text(stmt, 2);

            msg.sender = sText ? reinterpret_cast<const char*>(sText) : "";
            msg.receiver = rText ? reinterpret_cast<const char*>(rText) : "";
            msg.content = cText ? reinterpret_cast<const char*>(cText) : "";
            msg.timestamp = sqlite3_column_int64(stmt, 3);

            history.push_back(msg);
        }
        sqlite3_finalize(stmt);
    }
    return history;
}