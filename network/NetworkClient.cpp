#include "NetworkClient.h"
#include <iostream>
#include <cstring>
#include <chrono>

namespace chessgame::network {

NetworkClient::NetworkClient() : clientSocket(-1), connected(false), running(false), heartbeatRunning(false) {
}

NetworkClient::~NetworkClient() {
    disconnect();
}

bool NetworkClient::connect(const std::string& serverIP, int port) {
    // 创建socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "创建客户端socket失败" << std::endl;
        return false;
    }
    
    // 设置服务器地址
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        std::cerr << "无效的服务器IP地址: " << serverIP << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    // 连接到服务器
    if (::connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "连接服务器失败: " << serverIP << ":" << port << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    // 等待服务器确认
    NetworkMessage response = receiveMessage();
    if (response.type != MessageType::CONNECT_RESPONSE || response.data != "OK") {
        std::cerr << "服务器连接确认失败" << std::endl;
        close(clientSocket);
        clientSocket = -1;
        return false;
    }
    
    connected = true;
    running = true;
    
    // 启动接收消息线程
    receiveThread = std::thread(&NetworkClient::receiveMessages, this);
    
    // 启动心跳线程
    startHeartbeat();
    
    std::cout << "成功连接到服务器: " << serverIP << ":" << port << std::endl;
    
    // 调用连接回调
    if (connectCallback) {
        connectCallback();
    }
    
    return true;
}

void NetworkClient::disconnect() {
    if (!connected.load()) return;
    
    // 发送断开连接消息
    NetworkMessage disconnectMsg(MessageType::DISCONNECT, "BYE");
    sendMessage(disconnectMsg);
    
    connected = false;
    running = false;
    heartbeatRunning = false;
    
    // 等待线程结束
    if (receiveThread.joinable()) {
        receiveThread.join();
    }
    
    if (heartbeatThread.joinable()) {
        heartbeatThread.join();
    }
    
    // 关闭socket
    if (clientSocket >= 0) {
        close(clientSocket);
        clientSocket = -1;
    }
    
    std::cout << "已断开与服务器的连接" << std::endl;
    
    // 调用断开连接回调
    if (disconnectCallback) {
        disconnectCallback();
    }
}

void NetworkClient::receiveMessages() {
    while (running.load() && connected.load()) {
        NetworkMessage message = receiveMessage();
        
        if (message.type == MessageType::ERROR) {
            std::cerr << "接收消息错误，断开连接" << std::endl;
            break;
        }
        
        // 处理心跳响应
        if (message.type == MessageType::HEARTBEAT && message.data == "PONG") {
            continue;
        }
        
        // 调用消息回调
        if (messageCallback) {
            messageCallback(message);
        }
    }
    
    connected = false;
    
    // 调用断开连接回调
    if (disconnectCallback) {
        disconnectCallback();
    }
}

bool NetworkClient::sendMessageInternal(const NetworkMessage& message) {
    if (!connected.load()) return false;
    
    std::string serialized = message.serialize();
    
    // 添加长度前缀
    std::string lengthPrefix = NetworkMessage::createLengthPrefix(serialized.length());
    std::string fullMessage = lengthPrefix + serialized;
    
    ssize_t sent = send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
    return sent == static_cast<ssize_t>(fullMessage.length());
}

bool NetworkClient::sendMessage(const NetworkMessage& message) {
    if (!connected.load()) return false;
    return sendMessageInternal(message);
}

NetworkMessage NetworkClient::receiveMessage() {
    // 首先读取长度前缀
    char lengthBuffer[9] = {0}; // 8位长度 + 1位结束符
    ssize_t received = recv(clientSocket, lengthBuffer, 8, 0);
    
    if (received <= 0) {
        return NetworkMessage(MessageType::ERROR, "Connection lost");
    }
    
    size_t messageLength = NetworkMessage::parseLengthPrefix(std::string(lengthBuffer, 8));
    if (messageLength == 0 || messageLength > NetworkConfig::BUFFER_SIZE) {
        return NetworkMessage(MessageType::ERROR, "Invalid message length");
    }
    
    // 读取消息内容
    char messageBuffer[NetworkConfig::BUFFER_SIZE] = {0};
    size_t totalReceived = 0;
    
    while (totalReceived < messageLength) {
        received = recv(clientSocket, messageBuffer + totalReceived, 
                       messageLength - totalReceived, 0);
        
        if (received <= 0) {
            return NetworkMessage(MessageType::ERROR, "Connection lost");
        }
        
        totalReceived += received;
    }
    
    std::string messageData(messageBuffer, messageLength);
    return NetworkMessage::deserialize(messageData);
}

void NetworkClient::startHeartbeat() {
    heartbeatRunning = true;
    heartbeatThread = std::thread([this]() {
        while (heartbeatRunning.load() && connected.load()) {
            // 发送心跳
            NetworkMessage heartbeat(MessageType::HEARTBEAT, "PING");
            if (!sendMessage(heartbeat)) {
                break;
            }
            
            // 等待心跳间隔
            std::this_thread::sleep_for(std::chrono::seconds(NetworkConfig::HEARTBEAT_INTERVAL));
        }
    });
}

void NetworkClient::stopHeartbeat() {
    heartbeatRunning = false;
    if (heartbeatThread.joinable()) {
        heartbeatThread.join();
    }
}

} // namespace chessgame::network