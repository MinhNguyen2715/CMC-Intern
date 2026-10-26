#pragma once
#include <string>
#include "sqlite3.h"
#include <vector>
#include <cstdint>
#include <mutex>

struct ChatMessage {
    std::string sender;
    std::string receiver;
    std::string content;
    int64_t timestamp;
};

class DatabaseManager {
private:
    sqlite3* m_pDB;
    std::mutex m_dbMutex;

public:
    DatabaseManager();
    ~DatabaseManager();

    void Close();
    bool InitDatabase(const std::string& dbPath);
    bool RegisterUser(const std::string& username, const std::string& password);
    bool AuthenticateUser(const std::string& username, const std::string& password);
    bool SaveMessage(const std::string& sender, const std::string& receiver, const std::string& content, int64_t timestamp = 0);
    std::vector<std::string> GetAllUsers(const std::string& currentUsername);
    std::vector<ChatMessage> GetChatHistory(const std::string& user1, const std::string& user2);
};