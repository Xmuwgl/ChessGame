#include "NetworkProtocol.h"
#include <sstream>
#include <iomanip>

namespace chessgame::network {

std::string NetworkMessage::serialize() const {
    std::ostringstream oss;
    oss << static_cast<int>(type) << ":" << data.length() << ":" << data;
    return oss.str();
}

NetworkMessage NetworkMessage::deserialize(const std::string& str) {
    size_t pos1 = str.find(':');
    size_t pos2 = str.find(':', pos1 + 1);
    
    if (pos1 == std::string::npos || pos2 == std::string::npos) {
        return NetworkMessage(MessageType::ERROR, "Invalid message format");
    }
    
    int type = std::stoi(str.substr(0, pos1));
    size_t length = std::stoull(str.substr(pos1 + 1, pos2 - pos1 - 1));
    std::string data = str.substr(pos2 + 1, length);
    
    return NetworkMessage(static_cast<MessageType>(type), data);
}

std::string NetworkMessage::createLengthPrefix(size_t length) {
    std::ostringstream oss;
    oss << std::setw(8) << std::setfill('0') << length;
    return oss.str();
}

size_t NetworkMessage::parseLengthPrefix(const std::string& prefix) {
    if (prefix.length() < 8) return 0;
    return std::stoull(prefix);
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
    
    if (std::getline(iss, token, ',')) {
        move.row = std::stoi(token);
    }
    if (std::getline(iss, token, ',')) {
        move.col = std::stoi(token);
    }
    if (std::getline(iss, token, ',')) {
        move.player = static_cast<PieceType>(std::stoi(token));
    }
    
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
    if (std::getline(iss, token, ',')) {
        state.boardState = token;
    }
    
    return state;
}

} // namespace chessgame::network