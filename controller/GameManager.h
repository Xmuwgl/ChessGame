#pragma once

#include "../utils/Type.h"
#include "../facade/GameFacade.h"
#include "../view/GameView.h"
#include "Command.h"
#include "../ai/AI.h"
#include "../recording/GameRecorder.h"
#include "../account/AccountManager.h"
#include "../network/NetworkServer.h"
#include "../network/NetworkClient.h"
#include "../network/NetworkProtocol.h"
#include <memory>
#include <vector>

namespace chessgame::controller {

// 游戏模式枚举
enum class GameMode {
    PVP,    // 玩家对玩家
    PVAI,   // 玩家对AI
    AIVAI,  // AI对AI
    NETWORK // 网络对战
};

class GameManager {
private:
    std::unique_ptr<facade::GameFacade> gameFacade;
    std::unique_ptr<view::GameView> gameView;
    std::vector<std::unique_ptr<Command>> commandHistory;
    
    // AI玩家
    std::unique_ptr<ai::AIPlayer> blackAI;
    std::unique_ptr<ai::AIPlayer> whiteAI;
    
    // 游戏模式
    GameMode gameMode;
    
    // 录像相关
    std::unique_ptr<recording::GameRecord> currentRecord;
    recording::RecordManager recordManager;
    bool isRecording;
    
    // 账户相关
    account::AccountManager accountManager;
    account::LeaderboardManager leaderboardManager;
    
    // 网络相关
    std::unique_ptr<network::NetworkServer> networkServer;
    std::unique_ptr<network::NetworkClient> networkClient;
    bool isHost;
    bool isNetworkGame;
    int networkPlayerType; // 1: 黑棋, 2: 白棋
    bool gameInitialized = false;
    
    // 网络游戏回合管理（参考 GoBang）
    PieceType selfPieceType;  // 自己的棋子类型
    int restTime;             // 剩余时间（秒）
    int selfTotalTimeUsed;    // 自己总用时
    int opponentTotalTimeUsed;// 对手总用时
    int undoRestTime;         // 剩余悔棋次数
    static const int TURN_MAX_TIME = 30;  // 每回合最大时间
    std::chrono::steady_clock::time_point turnStartTime;  // 回合开始时间
    
    // 通知消息处理状态
    bool readyToQuit;
    bool readyToUndo;
    bool readyToLoad;
    
    bool running;
    
    // 解析用户输入并创建命令
    std::unique_ptr<Command> parseCommand(const std::string& input);
    
    // 执行命令
    bool executeCommand(std::unique_ptr<Command> command);
    
    // 初始化游戏
    bool initializeGame();
    
    // 初始化游戏模式
    bool initializeGameMode();
    
    // 初始化AI玩家
    void initializeAIPlayers();
    
    // 游戏主循环
    void gameLoop();
    
    // AI回合
    void aiTurn();
    
    // 检查当前玩家是否是AI
    bool isCurrentPlayerAI() const;
    
    // 获取当前AI玩家
    ai::AIPlayer* getCurrentAIPlayer() const;
    
    // 获取玩家名称
    std::string getPlayerName(PieceType player) const;
    
    // 获取玩家战绩信息
    std::string getPlayerStats(PieceType player) const;
    
    // 开始录像
    void startRecording();
    
    // 停止录像
    void stopRecording();
    
    // 保存录像
    bool saveRecording(const std::string& filename = "");
    
    // 加载录像
    bool loadRecording(const std::string& filename);
    
    // 回放录像
    void replayRecording(const std::string& filename);
    
    // 获取用户相关的录像列表
    std::vector<std::string> getUserRecordList() const;
    
    // 账户管理
    void showAccountMenu();
    void registerAccount();
    void login();
    void logout();
    void showUserStats();
    void showLeaderboard();
    void changePassword();
    void deleteAccount();
    
    // 网络对战菜单
    void showNetworkMenu();
    
    // 网络游戏主循环
    void networkGameLoop();
    
    // 网络相关方法
    void startNetworkServer();
    void connectToServer(const std::string& serverIP);
    void stopNetworkGame();
    bool initializeNetworkGame();
    void handleNetworkMessage(int clientSocket, const network::NetworkMessage& message);
    void handleClientMessage(const network::NetworkMessage& message);
    void sendNetworkMove(int row, int col);
    void sendGameState();
    void syncGameState(const network::GameStateInfo& state);
    void handleNetworkDisconnection();
    std::string getGameTypeName(GameType type);
    
    // 网络消息处理（参考 GoBang 的 Operate 方法）
    void operateNetworkMessage(const network::NetworkMessage& message, PieceType playerState);
    void notifyMessageHandler(const network::NotifyInfo& notifyInfo, PieceType playerState);
    void takeTurn();  // 切换回合
    void timeTick();  // 时间计数
    void checkTimeout();  // 检查超时
    void startTurnTimer();  // 开始回合计时
    void stopTurnTimer();   // 停止回合计时

public:
    GameManager();
    ~GameManager() = default;
    
    // 开始游戏
    void startGame();
    
    // 重新开始游戏
    void restartGame();
    
    // 退出游戏
    void quitGame();
    
    // 处理游戏结束
    void handleGameEnd(GameStatus status);
};

} // namespace chessgame::controller