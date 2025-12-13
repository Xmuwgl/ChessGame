#include "NetworkProtocol.h"
#include <sstream>
#include <iomanip>

namespace chessgame::network {

std::string NetworkMessage::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << ":" << data;
    return oss.str();
}

NetworkMessage NetworkMessage::deserialize(const std::string& str) {
    size_t pos1 = str.find(':');
    
    if (pos1 == std::string::npos)
        return NetworkMessage(MessageType::ERROR, "Invalid message format");
    
    int type = std::stoi(str.substr(0, pos1));
    std::string data = str.substr(pos1 + 1);
    
    return NetworkMessage(static_cast<MessageType>(type), data);
}

std::string NetworkMessage::createLengthPrefix(size_t length) {
    std::ostringstream oss;
    oss << std::setw(8) << std::setfill('0') << length;
    return oss.str();
}

size_t NetworkMessage::parseLengthPrefix(const std::string& prefix) {
    if (prefix.length() < 8) return 0;
    try {
        return std::stoull(prefix);
    } catch (const std::exception& e) {
        return 0;
    }
}

std::string MoveInfo::serialize() const {
    std::ostringstream oss;
    oss << row << "," << col << "," << static_cast<int>(player);
    return oss.str();
}

MoveInfo MoveInfo::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    
    MoveInfo move;
    
    if (std::getline(iss, token, ',')) move.row = std::stoi(token);
    if (std::getline(iss, token, ',')) move.col = std::stoi(token);
    if (std::getline(iss, token, ',')) move.player = static_cast<PieceType>(std::stoi(token));
    
    return move;
}

std::string GameStateInfo::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(gameType) << ","
        << static_cast<int>(currentPlayer) << ","
        << static_cast<int>(gameStatus) << ","
        << boardState;
    return oss.str();
}

GameStateInfo GameStateInfo::deserialize(const std::string& data) {
    std::istringstream iss(data);
    std::string token;
    
    GameStateInfo state;
    
    if (std::getline(iss, token, ',')) {
        state.gameType = static_cast<GameType>(std::stoi(token));
    }
    if (std::getline(iss, token, ',')) {
        state.currentPlayer = static_cast<PieceType>(std::stoi(token));
    }
    if (std::getline(iss, token, ',')) {
        state.gameStatus = static_cast<GameStatus>(std::stoi(token));
    }
    // 剩余的所有内容都是棋盘状态
    std::getline(iss, token);
    state.boardState = token;
    
    return state;
}

std::string NotifyInfo::serialize() const {
    return std::to_string(static_cast<int>(notifyType));
}

NotifyInfo NotifyInfo::deserialize(const std::string& data) {
    NotifyInfo info;
    info.notifyType = static_cast<NotifyType>(std::stoi(data));
    return info;
}

std::string TimeCountInfo::serialize() const {
    return std::to_string(timeCount);
}

TimeCountInfo TimeCountInfo::deserialize(const std::string& data) {
    TimeCountInfo info;
    info.timeCount = std::stoi(data);
    return info;
}

} // namespace chessgame::network