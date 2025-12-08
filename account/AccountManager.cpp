#include "AccountManager.h"
#include "../utils/Type.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <iomanip>

namespace chessgame::account {

// GameStats实现
void GameStats::updateStats(bool isWin, bool isDraw, chessgame::GameType gameType, bool isAI) {
    totalGames++;
    lastPlayTime = std::time(nullptr);
    
    if (isDraw) {
        draws++;
    } else if (isWin) {
        wins++;
        
        // 按游戏类型统计
        switch (gameType) {
            case GOMOKU: gomokuWins++; break;
            case GO: goWins++; break;
            case OTHELLO: othelloWins++; break;
            default: break;
        }
        
        // 按对手类型统计
        if (isAI) {
            aiWins++;
        } else {
            humanWins++;
        }
    } else {
        losses++;
    }
    
    calculateWinRate();
}

void GameStats::calculateWinRate() {
    if (totalGames > 0) {
        winRate = static_cast<double>(wins) / totalGames * 100.0;
    } else {
        winRate = 0.0;
    }
}

// UserAccount实现
UserAccount::UserAccount(const std::string& name, const std::string& pwd) 
    : username(name), password(pwd), createTime(std::time(nullptr)), 
      lastLoginTime(std::time(nullptr)) {
}

const std::string& UserAccount::getUsername() const {
    return username;
}

bool UserAccount::verifyPassword(const std::string& pwd) const {
    return password == pwd;
}

void UserAccount::changePassword(const std::string& newPwd) {
    password = newPwd;
}

std::time_t UserAccount::getCreateTime() const {
    return createTime;
}

std::time_t UserAccount::getLastLoginTime() const {
    return lastLoginTime;
}

void UserAccount::updateLastLoginTime() {
    lastLoginTime = std::time(nullptr);
}

const GameStats& UserAccount::getStats() const {
    return stats;
}

void UserAccount::updateGameStats(bool isWin, bool isDraw, GameType gameType, bool isAI) {
    stats.updateStats(isWin, isDraw, gameType, isAI);
}

const std::vector<std::string>& UserAccount::getSavedGames() const {
    return savedGames;
}

void UserAccount::addSavedGame(const std::string& filename) {
    // 检查是否已存在
    for (const auto& game : savedGames) {
        if (game == filename) {
            return;  // 已存在，不重复添加
        }
    }
    
    savedGames.push_back(filename);
}

void UserAccount::removeSavedGame(const std::string& filename) {
    savedGames.erase(
        std::remove(savedGames.begin(), savedGames.end(), filename),
        savedGames.end()
    );
}

bool UserAccount::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // 写入基本信息
    file << "Username: " << username << "\n";
    file << "Password: " << password << "\n";
    file << "CreateTime: " << createTime << "\n";
    file << "LastLoginTime: " << lastLoginTime << "\n";
    
    // 写入统计信息
    file << "TotalGames: " << stats.totalGames << "\n";
    file << "Wins: " << stats.wins << "\n";
    file << "Losses: " << stats.losses << "\n";
    file << "Draws: " << stats.draws << "\n";
    file << "GomokuWins: " << stats.gomokuWins << "\n";
    file << "GoWins: " << stats.goWins << "\n";
    file << "OthelloWins: " << stats.othelloWins << "\n";
    file << "AIWins: " << stats.aiWins << "\n";
    file << "HumanWins: " << stats.humanWins << "\n";
    file << "WinRate: " << stats.winRate << "\n";
    file << "LastPlayTime: " << stats.lastPlayTime << "\n";
    
    // 写入保存的游戏列表
    file << "SavedGamesCount: " << savedGames.size() << "\n";
    for (const auto& game : savedGames) {
        file << "SavedGame: " << game << "\n";
    }
    
    file.close();
    return true;
}

bool UserAccount::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        
        if (!(iss >> key)) {
            continue;
        }
        
        if (key == "Username:") {
            std::getline(iss, username);
            if (!username.empty() && username[0] == ' ') {
                username = username.substr(1);
            }
        } else if (key == "Password:") {
            std::getline(iss, password);
            if (!password.empty() && password[0] == ' ') {
                password = password.substr(1);
            }
        } else if (key == "CreateTime:") {
            iss >> createTime;
        } else if (key == "LastLoginTime:") {
            iss >> lastLoginTime;
        } else if (key == "TotalGames:") {
            iss >> stats.totalGames;
        } else if (key == "Wins:") {
            iss >> stats.wins;
        } else if (key == "Losses:") {
            iss >> stats.losses;
        } else if (key == "Draws:") {
            iss >> stats.draws;
        } else if (key == "GomokuWins:") {
            iss >> stats.gomokuWins;
        } else if (key == "GoWins:") {
            iss >> stats.goWins;
        } else if (key == "OthelloWins:") {
            iss >> stats.othelloWins;
        } else if (key == "AIWins:") {
            iss >> stats.aiWins;
        } else if (key == "HumanWins:") {
            iss >> stats.humanWins;
        } else if (key == "WinRate:") {
            iss >> stats.winRate;
        } else if (key == "LastPlayTime:") {
            iss >> stats.lastPlayTime;
        } else if (key == "SavedGame:") {
            std::string gameFile;
            std::getline(iss, gameFile);
            if (!gameFile.empty() && gameFile[0] == ' ') {
                gameFile = gameFile.substr(1);
            }
            savedGames.push_back(gameFile);
        }
    }
    
    file.close();
    return true;
}

// AccountManager实现
AccountManager::AccountManager(const std::string& dir) : accountDirectory(dir) {
    // 确保账户目录存在
    if (!std::filesystem::exists(accountDirectory)) {
        std::filesystem::create_directories(accountDirectory);
    }
    
    // 加载所有账户
    loadAllAccounts();
}

bool AccountManager::registerAccount(const std::string& username, const std::string& password) {
    // 检查用户名是否已存在
    if (userExists(username)) {
        return false;
    }
    
    // 创建新账户
    auto account = std::make_unique<UserAccount>(username, password);
    std::string filename = accountDirectory + "/" + username + ".txt";
    
    // 保存账户信息
    if (account->saveToFile(filename)) {
        accounts[username] = std::move(account);
        return true;
    }
    
    return false;
}

bool AccountManager::login(const std::string& username, const std::string& password) {
    auto it = accounts.find(username);
    if (it != accounts.end()) {
        if (it->second->verifyPassword(password)) {
            it->second->updateLastLoginTime();
            currentUser = username;
            return true;
        }
    }
    return false;
}

void AccountManager::logout() {
    currentUser = "";
}

bool AccountManager::isLoggedIn() const {
    return !currentUser.empty();
}

const std::string& AccountManager::getCurrentUser() const {
    return currentUser;
}

UserAccount* AccountManager::getCurrentUserAccount() const {
    if (!isLoggedIn()) {
        return nullptr;
    }
    
    auto it = accounts.find(currentUser);
    if (it != accounts.end()) {
        return it->second.get();
    }
    
    return nullptr;
}

const UserAccount* AccountManager::getUserAccount(const std::string& username) const {
    auto it = accounts.find(username);
    if (it != accounts.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool AccountManager::userExists(const std::string& username) const {
    return accounts.find(username) != accounts.end();
}

std::vector<std::string> AccountManager::getAllUsernames() const {
    std::vector<std::string> usernames;
    for (const auto& pair : accounts) {
        usernames.push_back(pair.first);
    }
    return usernames;
}

bool AccountManager::deleteAccount(const std::string& username, const std::string& password) {
    auto it = accounts.find(username);
    if (it != accounts.end()) {
        if (it->second->verifyPassword(password)) {
            // 删除文件
            std::string filename = accountDirectory + "/" + username + ".txt";
            std::filesystem::remove(filename);
            
            // 从内存中删除
            accounts.erase(it);
            
            // 如果删除的是当前登录用户，登出
            if (currentUser == username) {
                logout();
            }
            
            return true;
        }
    }
    return false;
}

bool AccountManager::changePassword(const std::string& username, const std::string& oldPassword, 
                                   const std::string& newPassword) {
    auto it = accounts.find(username);
    if (it != accounts.end()) {
        if (it->second->verifyPassword(oldPassword)) {
            it->second->changePassword(newPassword);
            
            // 保存更新后的账户信息
            std::string filename = accountDirectory + "/" + username + ".txt";
            return it->second->saveToFile(filename);
        }
    }
    return false;
}

bool AccountManager::saveAllAccounts() const {
    for (const auto& pair : accounts) {
        std::string filename = accountDirectory + "/" + pair.first + ".txt";
        if (!pair.second->saveToFile(filename)) {
            return false;
        }
    }
    return true;
}

bool AccountManager::loadAllAccounts() {
    accounts.clear();
    
    if (!std::filesystem::exists(accountDirectory)) {
        return false;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(accountDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            std::string filename = entry.path().string();
            auto account = std::make_unique<UserAccount>("", "");  // 临时账户
            
            if (account->loadFromFile(filename)) {
                std::string username = account->getUsername();
                if (!username.empty()) {
                    accounts[username] = std::move(account);
                }
            }
        }
    }
    
    return true;
}

void AccountManager::setAccountDirectory(const std::string& dir) {
    accountDirectory = dir;
    // 确保新目录存在
    if (!std::filesystem::exists(accountDirectory)) {
        std::filesystem::create_directories(accountDirectory);
    }
    
    // 重新加载账户
    loadAllAccounts();
}

const std::string& AccountManager::getAccountDirectory() const {
    return accountDirectory;
}

// LeaderboardManager实现
LeaderboardManager::LeaderboardManager(AccountManager* manager) : accountManager(manager) {
    updateLeaderboard();
}

void LeaderboardManager::updateLeaderboard() {
    entries.clear();
    
    if (!accountManager) {
        return;
    }
    
    auto usernames = accountManager->getAllUsernames();
    for (const auto& username : usernames) {
        // 获取用户账户
        const UserAccount* account = accountManager->getUserAccount(username);
        if (account != nullptr) {
            const GameStats& stats = account->getStats();
            entries.emplace_back(username, stats.wins, stats.winRate, stats.totalGames);
        }
    }
    
    // 按胜利次数排序，如果相同则按胜率排序
    std::sort(entries.begin(), entries.end(), 
              [](const LeaderboardEntry& a, const LeaderboardEntry& b) {
                  if (a.wins != b.wins) {
                      return a.wins > b.wins;
                  }
                  return a.winRate > b.winRate;
              });
}

const std::vector<LeaderboardEntry>& LeaderboardManager::getLeaderboard() const {
    return entries;
}

int LeaderboardManager::getUserRank(const std::string& username) const {
    for (size_t i = 0; i < entries.size(); i++) {
        if (entries[i].username == username) {
            return static_cast<int>(i) + 1;
        }
    }
    return -1;  // 未找到
}

void LeaderboardManager::displayLeaderboard() const {
    std::cout << std::endl;
    std::cout << "==================== 排行榜 ====================" << std::endl;
    std::cout << std::setw(5) << "排名" << " | " 
              << std::setw(15) << "用户名" << " | "
              << std::setw(8) << "胜利次数" << " | "
              << std::setw(8) << "总场次" << " | "
              << std::setw(8) << "胜率(%)" << std::endl;
    std::cout << "------------------------------------------------" << std::endl;
    
    for (size_t i = 0; i < entries.size(); i++) {
        const auto& entry = entries[i];
        std::cout << std::setw(5) << (i + 1) << " | "
                  << std::setw(15) << entry.username << " | "
                  << std::setw(8) << entry.wins << " | "
                  << std::setw(8) << entry.totalGames << " | "
                  << std::setw(8) << std::fixed << std::setprecision(1) << entry.winRate << std::endl;
    }
    
    std::cout << "================================================" << std::endl;
    std::cout << std::endl;
}

}