#include "Command.h"
#include "../facade/GameFacade.h"
#include <sstream>

using namespace chessgame::controller;

MoveCommand::MoveCommand(facade::GameFacade* facade, int posX, int posY, PieceType p) 
    : Command(facade), x(posX), y(posY), player(p), wasExecuted(false) {}

bool MoveCommand::execute() {
    if (gameFacade->makeMove(x, y, player)) {
        wasExecuted = true;
        return true;
    }
    return false;
}

bool MoveCommand::undo() {
    if (!wasExecuted) return false;
    return gameFacade->undoMove();
}

std::string MoveCommand::getDescription() const {
    std::stringstream ss;
    ss << "落子 (" << x + 1 << ", " << y + 1 << ") - " << (player == BLACK ? "黑方" : "白方");
    return ss.str();
}

PassCommand::PassCommand(facade::GameFacade* facade, PieceType p) 
    : Command(facade), player(p), wasExecuted(false) {}

bool PassCommand::execute() {
    if (gameFacade->passMove(player)) {
        wasExecuted = true;
        return true;
    }
    return false;
}

bool PassCommand::undo() {
    if (!wasExecuted) return false;
    return gameFacade->undoMove();
}

std::string PassCommand::getDescription() const {
    return "虚着 - " + std::string(player == BLACK ? "黑方" : "白方");
}

UndoCommand::UndoCommand(facade::GameFacade* facade) : Command(facade) {}

bool UndoCommand::execute() {
    return gameFacade->undoMove();
}

bool UndoCommand::undo() {
    // 撤销"撤销"操作：重新执行上一步
    // 这里简化处理，实际应用中可能需要更复杂的实现
    return false;
}

std::string UndoCommand::getDescription() const {
    return "悔棋";
}

ResignCommand::ResignCommand(facade::GameFacade* facade, PieceType p) 
    : Command(facade), player(p) {}

bool ResignCommand::execute() {
    return gameFacade->resign(player);
}

bool ResignCommand::undo() {
    // 认输操作通常不可撤销
    return false;
}

std::string ResignCommand::getDescription() const {
    return "认输 - " + std::string(player == BLACK ? "黑方" : "白方");
}

SaveCommand::SaveCommand(facade::GameFacade* facade, const std::string& file) 
    : Command(facade), filename(file) {}

bool SaveCommand::execute() {
    return gameFacade->saveGame(filename);
}

bool SaveCommand::undo() {
    // 保存操作通常不可撤销
    return false;
}

std::string SaveCommand::getDescription() const {
    return "保存游戏到 " + filename;
}

LoadCommand::LoadCommand(facade::GameFacade* facade, const std::string& file) 
    : Command(facade), filename(file) {}

bool LoadCommand::execute() {
    return gameFacade->loadGame(filename);
}

bool LoadCommand::undo() {
    // 加载操作通常不可撤销
    return false;
}

std::string LoadCommand::getDescription() const {
    return "从 " + filename + " 加载游戏";
}

RestartCommand::RestartCommand(facade::GameFacade* facade) : Command(facade) {}

bool RestartCommand::execute() {
    return gameFacade->restartGame();
}

bool RestartCommand::undo() {
    // 重新开始操作通常不可撤销
    return false;
}

std::string RestartCommand::getDescription() const {
    return "重新开始游戏";
}
