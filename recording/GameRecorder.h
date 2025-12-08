#pragma once
#include "../utils/Type.h"
#include <vector>
#include <string>
#include <memory>
#include <fstream>

namespace chessgame::recording {

// 录像条目，记录一步棋
struct GameRecordEntry {
    Move move;              // 移动信息
    std::string timestamp;   // 时间戳
    std::string note;        // 备注
    
    GameRecordEntry(const Move& m, const std::string& t = "", const std::string& n = "")
        : move(m), timestamp(t), note(n) {}
};

// 游戏录像
class GameRecord {
private:
    GameType gameType;       // 游戏类型
    int boardSize;           // 棋盘大小
    std::vector<GameRecordEntry> entries;  // 录像条目
    std::string blackPlayer; // 黑方玩家名
    std::string whitePlayer; // 白方玩家名
    GameStatus result;       // 游戏结果
    std::string recordDate;  // 录像日期
    
public:
    GameRecord(GameType type, int size, const std::string& black = "Black", 
               const std::string& white = "White");
    ~GameRecord() = default;
    
    // 添加一步棋到录像
    void addMove(const Move& move, const std::string& note = "");
    
    // 设置游戏结果
    void setResult(GameStatus status);
    
    // 获取游戏类型
    GameType getGameType() const;
    
    // 获取棋盘大小
    int getBoardSize() const;
    
    // 获取录像条目
    const std::vector<GameRecordEntry>& getEntries() const;
    
    // 获取黑方玩家名
    const std::string& getBlackPlayer() const;
    
    // 获取白方玩家名
    const std::string& getWhitePlayer() const;
    
    // 获取游戏结果
    GameStatus getResult() const;
    
    // 获取录像日期
    const std::string& getRecordDate() const;
    
    // 设置玩家名
    void setPlayerNames(const std::string& black, const std::string& white);
    
    // 保存录像到文件
    bool saveToFile(const std::string& filename) const;
    
    // 从文件加载录像
    bool loadFromFile(const std::string& filename);
    
    // 清空录像
    void clear();
};

// 录像管理器
class RecordManager {
private:
    std::vector<std::unique_ptr<GameRecord>> records;
    std::string recordDirectory;  // 录像存储目录
    
public:
    RecordManager(const std::string& dir = "records");
    ~RecordManager() = default;
    
    // 创建新录像
    std::unique_ptr<GameRecord> createRecord(GameType type, int size, 
                                           const std::string& black = "Black", 
                                           const std::string& white = "White");
    
    // 保存录像
    bool saveRecord(const GameRecord& record, const std::string& filename = "");
    
    // 加载录像
    std::unique_ptr<GameRecord> loadRecord(const std::string& filename);
    
    // 获取所有录像文件列表
    std::vector<std::string> getRecordList() const;
    
    // 删除录像文件
    bool deleteRecord(const std::string& filename);
    
    // 设置录像目录
    void setRecordDirectory(const std::string& dir);
    
    // 获取录像目录
    const std::string& getRecordDirectory() const;
};

// 回放器
class GameReplayer {
private:
    std::unique_ptr<GameRecord> record;
    size_t currentMoveIndex;
    bool isReplaying;
    
public:
    GameReplayer();
    ~GameReplayer() = default;
    
    // 加载录像
    bool loadRecord(const std::string& filename);
    
    // 加载录像对象
    void loadRecordObject(std::unique_ptr<GameRecord> rec);
    
    // 开始回放
    void startReplay();
    
    // 下一步
    bool nextMove();
    
    // 上一步
    bool previousMove();
    
    // 跳转到指定步数
    bool jumpToMove(size_t moveIndex);
    
    // 获取当前步数
    size_t getCurrentMoveIndex() const;
    
    // 获取总步数
    size_t getTotalMoves() const;
    
    // 是否正在回放
    bool getIsReplaying() const;
    
    // 获取当前移动
    const GameRecordEntry* getCurrentMove() const;
    
    // 获取录像对象
    const GameRecord* getRecord() const;
    
    // 重置回放
    void resetReplay();
};

}