#include "GameRecorder.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include <ctime>

namespace chessgame::recording {

// GameRecord实现
GameRecord::GameRecord(GameType type, int size, const std::string& black, const std::string& white)
    : gameType(type), boardSize(size), blackPlayer(black), whitePlayer(white), result(IN_PROGRESS) {
    // 设置当前日期为录像日期
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    recordDate = oss.str();
}

void GameRecord::addMove(const Move& move, const std::string& note) {
    // 获取当前时间作为时间戳
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    std::string timestamp = oss.str();
    
    entries.emplace_back(move, timestamp, note);
}

void GameRecord::setResult(GameStatus status) {
    result = status;
}

GameType GameRecord::getGameType() const {
    return gameType;
}

int GameRecord::getBoardSize() const {
    return boardSize;
}

const std::vector<GameRecordEntry>& GameRecord::getEntries() const {
    return entries;
}

const std::string& GameRecord::getBlackPlayer() const {
    return blackPlayer;
}

const std::string& GameRecord::getWhitePlayer() const {
    return whitePlayer;
}

GameStatus GameRecord::getResult() const {
    return result;
}

const std::string& GameRecord::getRecordDate() const {
    return recordDate;
}

void GameRecord::setPlayerNames(const std::string& black, const std::string& white) {
    blackPlayer = black;
    whitePlayer = white;
}

bool GameRecord::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    // 写入游戏信息
    file << "GameType: " << static_cast<int>(gameType) << "\n";
    file << "BoardSize: " << boardSize << "\n";
    file << "BlackPlayer: " << blackPlayer << "\n";
    file << "WhitePlayer: " << whitePlayer << "\n";
    file << "Result: " << static_cast<int>(result) << "\n";
    file << "RecordDate: " << recordDate << "\n";
    file << "MoveCount: " << entries.size() << "\n";
    file << "-----Moves-----\n";
    
    // 写入每一步棋
    for (const auto& entry : entries) {
        file << "Move: " << entry.move.x << "," << entry.move.y << " ";
        file << "Player: " << static_cast<int>(entry.move.piece) << " ";
        file << "IsPass: " << (entry.move.isPass ? 1 : 0) << " ";
        file << "IsResign: " << (entry.move.isResign ? 1 : 0) << " ";
        file << "Timestamp: " << entry.timestamp << " ";
        file << "Note: " << entry.note << "\n";
    }
    
    file.close();
    return true;
}

bool GameRecord::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    
    entries.clear(); // 清空现有条目
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        
        if (!(iss >> key)) {
            continue;
        }
        
        if (key == "GameType:") {
            int type;
            iss >> type;
            gameType = static_cast<GameType>(type);
        } else if (key == "BoardSize:") {
            iss >> boardSize;
        } else if (key == "BlackPlayer:") {
            std::getline(iss, blackPlayer);
            // 去除前导空格
            if (!blackPlayer.empty() && blackPlayer[0] == ' ') {
                blackPlayer = blackPlayer.substr(1);
            }
        } else if (key == "WhitePlayer:") {
            std::getline(iss, whitePlayer);
            // 去除前导空格
            if (!whitePlayer.empty() && whitePlayer[0] == ' ') {
                whitePlayer = whitePlayer.substr(1);
            }
        } else if (key == "Result:") {
            int status;
            iss >> status;
            result = static_cast<GameStatus>(status);
        } else if (key == "RecordDate:") {
            std::getline(iss, recordDate);
            // 去除前导空格
            if (!recordDate.empty() && recordDate[0] == ' ') {
                recordDate = recordDate.substr(1);
            }
        } else if (key == "Move:") {
            // 解析移动信息
            Move move;
            std::string playerStr, isPassStr, isResignStr, timestampStr, noteStr;
            
            // 读取坐标
            std::string coordStr;
            iss >> coordStr;
            size_t commaPos = coordStr.find(',');
            if (commaPos != std::string::npos) {
                move.x = std::stoi(coordStr.substr(0, commaPos));
                move.y = std::stoi(coordStr.substr(commaPos + 1));
            }
            
            // 读取其他信息
            iss >> playerStr >> isPassStr >> isResignStr >> timestampStr;
            std::getline(iss, noteStr);
            
            // 转换
            if (playerStr.size() > 0) {
                move.piece = static_cast<PieceType>(std::stoi(playerStr.substr(playerStr.find(':') + 1)));
            }
            if (isPassStr.size() > 0) {
                move.isPass = (std::stoi(isPassStr.substr(isPassStr.find(':') + 1)) != 0);
            }
            if (isResignStr.size() > 0) {
                move.isResign = (std::stoi(isResignStr.substr(isResignStr.find(':') + 1)) != 0);
            }
            if (timestampStr.size() > 0) {
                std::string timestamp = timestampStr.substr(timestampStr.find(':') + 1);
            }
            if (noteStr.size() > 0) {
                std::string note = noteStr.substr(noteStr.find(':') + 1);
                // 去除前导空格
                if (!note.empty() && note[0] == ' ') {
                    note = note.substr(1);
                }
                entries.emplace_back(move, timestampStr, note);
            } else {
                entries.emplace_back(move, timestampStr, "");
            }
        }
    }
    
    file.close();
    return true;
}

void GameRecord::clear() {
    entries.clear();
    result = IN_PROGRESS;
}

// RecordManager实现
RecordManager::RecordManager(const std::string& dir) : recordDirectory(dir) {
    // 确保录像目录存在
    if (!std::filesystem::exists(recordDirectory)) {
        std::filesystem::create_directories(recordDirectory);
    }
}

std::unique_ptr<GameRecord> RecordManager::createRecord(GameType type, int size, 
                                                       const std::string& black, 
                                                       const std::string& white) {
    return std::make_unique<GameRecord>(type, size, black, white);
}

bool RecordManager::saveRecord(const GameRecord& record, const std::string& filename) {
    std::string file = filename;
    if (file.empty()) {
        // 生成默认文件名
        auto now = std::time(nullptr);
        auto tm = *std::localtime(&now);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
        
        std::string gameTypeStr;
        switch (record.getGameType()) {
            case GOMOKU: gameTypeStr = "Gomoku"; break;
            case GO: gameTypeStr = "Go"; break;
            case OTHELLO: gameTypeStr = "Othello"; break;
            default: gameTypeStr = "Unknown"; break;
        }
        
        file = recordDirectory + "/" + gameTypeStr + "_" + oss.str() + ".txt";
    }
    
    return record.saveToFile(file);
}

std::unique_ptr<GameRecord> RecordManager::loadRecord(const std::string& filename) {
    auto record = std::make_unique<GameRecord>(GOMOKU, 19); // 默认值，将被文件内容覆盖
    if (record->loadFromFile(filename)) {
        return record;
    }
    return nullptr;
}

std::vector<std::string> RecordManager::getRecordList() const {
    std::vector<std::string> records;
    
    if (!std::filesystem::exists(recordDirectory)) {
        return records;
    }
    
    for (const auto& entry : std::filesystem::directory_iterator(recordDirectory)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            records.push_back(entry.path().string());
        }
    }
    
    return records;
}

bool RecordManager::deleteRecord(const std::string& filename) {
    return std::filesystem::remove(filename);
}

void RecordManager::setRecordDirectory(const std::string& dir) {
    recordDirectory = dir;
    // 确保新目录存在
    if (!std::filesystem::exists(recordDirectory)) {
        std::filesystem::create_directories(recordDirectory);
    }
}

const std::string& RecordManager::getRecordDirectory() const {
    return recordDirectory;
}

// GameReplayer实现
GameReplayer::GameReplayer() : currentMoveIndex(0), isReplaying(false) {
}

bool GameReplayer::loadRecord(const std::string& filename) {
    RecordManager manager;
    auto rec = manager.loadRecord(filename);
    if (rec) {
        loadRecordObject(std::move(rec));
        return true;
    }
    return false;
}

void GameReplayer::loadRecordObject(std::unique_ptr<GameRecord> rec) {
    record = std::move(rec);
    currentMoveIndex = 0;
    isReplaying = false;
}

void GameReplayer::startReplay() {
    if (record) {
        isReplaying = true;
        currentMoveIndex = 0;
    }
}

bool GameReplayer::nextMove() {
    if (!record || !isReplaying || currentMoveIndex >= record->getEntries().size()) {
        return false;
    }
    
    currentMoveIndex++;
    return true;
}

bool GameReplayer::previousMove() {
    if (!record || !isReplaying || currentMoveIndex == 0) {
        return false;
    }
    
    currentMoveIndex--;
    return true;
}

bool GameReplayer::jumpToMove(size_t moveIndex) {
    if (!record || !isReplaying || moveIndex > record->getEntries().size()) {
        return false;
    }
    
    currentMoveIndex = moveIndex;
    return true;
}

size_t GameReplayer::getCurrentMoveIndex() const {
    return currentMoveIndex;
}

size_t GameReplayer::getTotalMoves() const {
    if (!record) {
        return 0;
    }
    return record->getEntries().size();
}

bool GameReplayer::getIsReplaying() const {
    return isReplaying;
}

const GameRecordEntry* GameReplayer::getCurrentMove() const {
    if (!record || !isReplaying || currentMoveIndex == 0 || currentMoveIndex > record->getEntries().size()) {
        return nullptr;
    }
    
    return &record->getEntries()[currentMoveIndex - 1];
}

const GameRecord* GameReplayer::getRecord() const {
    return record.get();
}

void GameReplayer::resetReplay() {
    if (record) {
        currentMoveIndex = 0;
        isReplaying = false;
    }
}

}