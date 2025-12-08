#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <ctime>
#include "../utils/Type.h"

namespace chessgame::account {

// 游戏统计信息
struct GameStats {
    int totalGames;         // 总游戏数
    int wins;              // 胜利次数
    int losses;            // 失败次数
    int draws;             // 平局次数
    int gomokuWins;        // 五子棋胜利次数
    int goWins;            // 围棋胜利次数
    int othelloWins;       // 黑白棋胜利次数
    int aiWins;            // 对AI胜利次数
    int humanWins;         // 对人类胜利次数
    double winRate;        // 胜率
    std::time_t lastPlayTime;  // 最后游戏时间
    
    GameStats() : totalGames(0), wins(0), losses(0), draws(0), 
                 gomokuWins(0), goWins(0), othelloWins(0), 
                 aiWins(0), humanWins(0), winRate(0.0), lastPlayTime(0) {}
    
    // 更新统计信息
    void updateStats(bool isWin, bool isDraw, chessgame::GameType gameType, bool isAI);
    
    // 计算胜率
    void calculateWinRate();
};

// 用户账户
class UserAccount {
private:
    std::string username;      // 用户名
    std::string password;      // 密码（简单存储，实际应用中应加密）
    std::time_t createTime;    // 创建时间
    std::time_t lastLoginTime; // 最后登录时间
    GameStats stats;           // 游戏统计
    std::vector<std::string> savedGames; // 保存的游戏录像文件名
    
public:
    UserAccount(const std::string& name, const std::string& pwd);
    ~UserAccount() = default;
    
    // 获取用户名
    const std::string& getUsername() const;
    
    // 验证密码
    bool verifyPassword(const std::string& pwd) const;
    
    // 更改密码
    void changePassword(const std::string& newPwd);
    
    // 获取创建时间
    std::time_t getCreateTime() const;
    
    // 获取最后登录时间
    std::time_t getLastLoginTime() const;
    
    // 更新最后登录时间
    void updateLastLoginTime();
    
    // 获取游戏统计
    const GameStats& getStats() const;
    
    // 更新游戏统计
    void updateGameStats(bool isWin, bool isDraw, chessgame::GameType gameType, bool isAI);
    
    // 获取保存的游戏列表
    const std::vector<std::string>& getSavedGames() const;
    
    // 添加保存的游戏
    void addSavedGame(const std::string& filename);
    
    // 移除保存的游戏
    void removeSavedGame(const std::string& filename);
    
    // 保存账户信息到文件
    bool saveToFile(const std::string& filename) const;
    
    // 从文件加载账户信息
    bool loadFromFile(const std::string& filename);
};

// 账户管理器
class AccountManager {
private:
    std::map<std::string, std::unique_ptr<UserAccount>> accounts;
    std::string currentUser;  // 当前登录用户
    std::string accountDirectory;  // 账户存储目录
    
public:
    AccountManager(const std::string& dir = "accounts");
    ~AccountManager() = default;
    
    // 注册新账户
    bool registerAccount(const std::string& username, const std::string& password);
    
    // 登录账户
    bool login(const std::string& username, const std::string& password);
    
    // 登出当前账户
    void logout();
    
    // 检查是否已登录
    bool isLoggedIn() const;
    
    // 获取当前用户名
    const std::string& getCurrentUser() const;
    
    // 获取当前用户账户
    UserAccount* getCurrentUserAccount() const;
    
    // 获取指定用户账户
    const UserAccount* getUserAccount(const std::string& username) const;
    
    // 检查用户名是否存在
    bool userExists(const std::string& username) const;
    
    // 获取所有用户名列表
    std::vector<std::string> getAllUsernames() const;
    
    // 删除账户
    bool deleteAccount(const std::string& username, const std::string& password);
    
    // 更改密码
    bool changePassword(const std::string& username, const std::string& oldPassword, 
                       const std::string& newPassword);
    
    // 保存所有账户
    bool saveAllAccounts() const;
    
    // 加载所有账户
    bool loadAllAccounts();
    
    // 设置账户目录
    void setAccountDirectory(const std::string& dir);
    
    // 获取账户目录
    const std::string& getAccountDirectory() const;
};

// 排行榜条目
struct LeaderboardEntry {
    std::string username;
    int wins;
    double winRate;
    int totalGames;
    
    LeaderboardEntry(const std::string& name, int w, double rate, int total)
        : username(name), wins(w), winRate(rate), totalGames(total) {}
};

// 排行榜管理器
class LeaderboardManager {
private:
    std::vector<LeaderboardEntry> entries;
    AccountManager* accountManager;
    
public:
    explicit LeaderboardManager(AccountManager* manager);
    ~LeaderboardManager() = default;
    
    // 更新排行榜
    void updateLeaderboard();
    
    // 获取排行榜
    const std::vector<LeaderboardEntry>& getLeaderboard() const;
    
    // 获取指定用户的排名
    int getUserRank(const std::string& username) const;
    
    // 显示排行榜
    void displayLeaderboard() const;
};

}