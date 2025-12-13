#pragma once
#include <string>
#include <vector>
#include "../utils/Type.h"

namespace chessgame::network {

// 网络消息类型枚举
enum class MessageType {
    // 连接管理
    CONNECT_REQUEST = 1001,
    CONNECT_RESPONSE = 1002,
    DISCONNECT = 1003,
    
    // 游戏设置
    GAME_START = 2001,
    GAME_TYPE_SELECT = 2002,
    PLAYER_ASSIGN = 2003,
    
    // 游戏操作
    MOVE = 3001,
    MOVE_RESPONSE = 3002,
    PASS = 3003,
    RESIGN = 3004,
    GAME_END = 3005,
    
    // 状态同步
    BOARD_SYNC = 4001,
    CURRENT_PLAYER_SYNC = 4002,
    GAME_STATUS_SYNC = 4003,
    
    // 通知消息（参考 GoBang）
    NOTIFY = 5001,
    TIMECOUNT = 5002,
    LOAD = 5003,
    CHAT = 5004,
    
    // 错误处理
    ERROR = 6001,
    HEARTBEAT = 6002
};

// 通知类型（参考 GoBang）
enum class NotifyType {
    NONE = 0,
    TIMEOUT = 1,    // 超时
    UNDO = 2,       // 悔棋请求
    QUIT = 3,       // 退出请求
    YES = 4,        // 同意
    NO = 5,         // 拒绝
    QLOAD = 6       // 请求加载
};

// 网络消息结构
struct NetworkMessage {
    MessageType type;
    std::string data;
    
    NetworkMessage(MessageType t, const std::string& d) : type(t), data(d) {}
    
    // 序列化为字符串
    std::string serialize() const;
    
    // 从字符串反序列化
    static NetworkMessage deserialize(const std::string& str);
    
    // 获取消息长度前缀
    static std::string createLengthPrefix(size_t length);
    
    // 解析消息长度
    static size_t parseLengthPrefix(const std::string& prefix);
};

// 游戏移动信息
struct MoveInfo {
    int row;
    int col;
    PieceType player;
    
    std::string serialize() const;
    static MoveInfo deserialize(const std::string& data);
};

// 游戏状态信息
struct GameStateInfo {
    GameType gameType;
    PieceType currentPlayer;
    GameStatus gameStatus;
    std::string boardState;
    
    std::string serialize() const;
    static GameStateInfo deserialize(const std::string& data);
};

// 通知消息信息
struct NotifyInfo {
    NotifyType notifyType;
    
    std::string serialize() const;
    static NotifyInfo deserialize(const std::string& data);
};

// 时间计数信息
struct TimeCountInfo {
    int timeCount;  // 使用的时间（秒）
    
    std::string serialize() const;
    static TimeCountInfo deserialize(const std::string& data);
};

// 网络配置
struct NetworkConfig {
    static const int DEFAULT_PORT = 12346;
    static const int MAX_CONNECTIONS = 2;
    static const int BUFFER_SIZE = 4096;
    static const int HEARTBEAT_INTERVAL = 30; // 秒
    static const int CONNECTION_TIMEOUT = 60; // 秒
};

// 错误代码
enum class NetworkError {
    NONE = 0,
    CONNECTION_FAILED = 1,
    INVALID_MESSAGE = 2,
    TIMEOUT = 3,
    DISCONNECTED = 4,
    PROTOCOL_ERROR = 5
};

} // namespace chessgame::network