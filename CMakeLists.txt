cmake_minimum_required(VERSION 3.10)
project(ChessGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC
  main.cpp
  controller/GameManager.cpp
  controller/Command.cpp
  facade/GameFacade.cpp
  model/Board.cpp
  model/GomokuRule.cpp
  model/GoRule.cpp
  model/Othello.cpp
  model/OthelloRule.cpp
  model/GameMemento.cpp
  view/GameView.cpp
  ai/AI.cpp
  ai/RandomAI.cpp
  ai/HeuristicAI.cpp
  account/AccountManager.cpp
  recording/GameRecorder.cpp
)

add_executable(game ${SRC})

target_compile_options(game PRIVATE -Wall -Wextra)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(game PRIVATE -O2)
endif()
