#pragma once
#include "../utils/Type.h"
#include "../facade/GameFacade.h"
#include <string>

namespace chessgame::controller {

class Command {
protected:
    facade::GameFacade* gameFacade;

public:
    Command(facade::GameFacade* facade) : gameFacade(facade) {}
    virtual ~Command() = default;
    
    virtual bool execute() = 0;
    virtual bool undo() = 0;
    virtual std::string getDescription() const = 0;
};

class MoveCommand : public Command {
private:
    int x, y;
    PieceType player;
    bool wasExecuted;

public:
    MoveCommand(facade::GameFacade* facade, int posX, int posY, PieceType p);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class PassCommand : public Command {
private:
    PieceType player;
    bool wasExecuted;

public:
    PassCommand(facade::GameFacade* facade, PieceType p);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class UndoCommand : public Command {
public:
    UndoCommand(facade::GameFacade* facade);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class ResignCommand : public Command {
private:
    PieceType player;

public:
    ResignCommand(facade::GameFacade* facade, PieceType p);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class SaveCommand : public Command {
private:
    std::string filename;

public:
    SaveCommand(facade::GameFacade* facade, const std::string& file);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class LoadCommand : public Command {
private:
    std::string filename;

public:
    LoadCommand(facade::GameFacade* facade, const std::string& file);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

class RestartCommand : public Command {
public:
    RestartCommand(facade::GameFacade* facade);
    
    bool execute() override;
    bool undo() override;
    std::string getDescription() const override;
};

}