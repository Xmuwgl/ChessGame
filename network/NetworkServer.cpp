#include "NetworkServer.h"
#include <iostream>
#include <cstring>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace chessgame::network {

NetworkServer::NetworkServer() : serverSocket(-1), running(false) {
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        clientSockets[i] = -1;
    }
}

NetworkServer::~NetworkServer() {
    stop();
}

bool NetworkServer::start(int port) {
    // 创建socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "创建服务器socket失败" << std::endl;
        return false;
    }
    
    // 设置socket选项
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "设置socket选项失败" << std::endl;
        close(serverSocket);
        return false;
    }
    
    // 绑定地址
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "绑定地址失败" << std::endl;
        close(serverSocket);
        return false;
    }
    
    // 开始监听
    if (listen(serverSocket, NetworkConfig::MAX_CONNECTIONS) < 0) {
        std::cerr << "监听失败" << std::endl;
        close(serverSocket);
        return false;
    }
    
    running = true;
    
    // 启动接受连接的线程
    acceptThread = std::thread(&NetworkServer::acceptConnections, this);
    
    std::cout << "服务器启动成功，监听端口: " << port << std::endl;
    std::cout << "本地IP地址: " << getLocalIPAddress() << std::endl;
    
    return true;
}

void NetworkServer::stop() {
    if (!running.load()) return;
    
    running = false;
    
    // 关闭所有客户端连接
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        if (clientSockets[i] >= 0) {
            close(clientSockets[i]);
            clientSockets[i] = -1;
        }
    }
    
    // 关闭服务器socket
    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }
    
    // 等待线程结束
    if (acceptThread.joinable()) {
        acceptThread.join();
    }
    
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        if (clientThreads[i].joinable()) {
            clientThreads[i].join();
        }
    }
    
    std::cout << "服务器已停止" << std::endl;
}

void NetworkServer::acceptConnections() {
    while (running.load()) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket < 0) {
            if (running.load()) {
                std::cerr << "接受连接失败" << std::endl;
            }
            continue;
        }
        
        // 查找空闲的客户端槽位
        int slot = -1;
        {
            std::lock_guard<std::mutex> lock(clientMutex);
            for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
                if (clientSockets[i] < 0) {
                    slot = i;
                    clientSockets[i] = clientSocket;
                    break;
                }
            }
        }
        
        if (slot == -1) {
            std::cerr << "服务器已满，拒绝连接" << std::endl;
            close(clientSocket);
            continue;
        }
        
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
        std::cout << "客户端连接: " << clientIP << ":" << ntohs(clientAddr.sin_port) 
                  << " (槽位: " << slot << ")" << std::endl;
        
        // 发送连接确认
        NetworkMessage response(MessageType::CONNECT_RESPONSE, "OK");
        sendMessage(clientSocket, response);
        
        // 调用连接回调
        if (connectCallback) {
            connectCallback(clientSocket);
        }
        
        // 启动客户端处理线程
        clientThreads[slot] = std::thread(&NetworkServer::handleClient, this, clientSocket);
    }
}

void NetworkServer::handleClient(int clientSocket) {
    while (running.load()) {
        NetworkMessage message = receiveMessage(clientSocket);
        
        if (message.type == MessageType::ERROR) {
            std::cerr << "接收消息错误，断开客户端连接" << std::endl;
            break;
        }
        
        // 处理心跳
        if (message.type == MessageType::HEARTBEAT) {
            NetworkMessage response(MessageType::HEARTBEAT, "PONG");
            sendMessage(clientSocket, response);
            continue;
        }
        
        // 调用消息回调
        if (messageCallback) {
            messageCallback(clientSocket, message);
        }
    }
    
    cleanupClient(clientSocket);
}

bool NetworkServer::sendMessage(int clientSocket, const NetworkMessage& message) {
    std::string serialized = message.serialize();
    
    // 添加长度前缀
    std::string lengthPrefix = NetworkMessage::createLengthPrefix(serialized.length());
    std::string fullMessage = lengthPrefix + serialized;
    
    ssize_t sent = send(clientSocket, fullMessage.c_str(), fullMessage.length(), 0);
    return sent == static_cast<ssize_t>(fullMessage.length());
}

NetworkMessage NetworkServer::receiveMessage(int clientSocket) {
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

void NetworkServer::cleanupClient(int clientSocket) {
    {
        std::lock_guard<std::mutex> lock(clientMutex);
        for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
            if (clientSockets[i] == clientSocket) {
                clientSockets[i] = -1;
                break;
            }
        }
    }
    
    close(clientSocket);
    
    // 调用断开连接回调
    if (disconnectCallback) {
        disconnectCallback(clientSocket);
    }
    
    std::cout << "客户端断开连接" << std::endl;
}

void NetworkServer::broadcastMessage(const NetworkMessage& message) {
    std::lock_guard<std::mutex> lock(clientMutex);
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        if (clientSockets[i] >= 0) {
            sendMessage(clientSockets[i], message);
        }
    }
}

void NetworkServer::sendToClient(int clientSocket, const NetworkMessage& message) {
    sendMessage(clientSocket, message);
}

int NetworkServer::getConnectedClientCount() const {
    int count = 0;
    std::lock_guard<std::mutex> lock(clientMutex);
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        if (clientSockets[i] >= 0) {
            count++;
        }
    }
    return count;
}

std::vector<int> NetworkServer::getConnectedClients() const {
    std::vector<int> clients;
    std::lock_guard<std::mutex> lock(clientMutex);
    for (int i = 0; i < NetworkConfig::MAX_CONNECTIONS; ++i) {
        if (clientSockets[i] >= 0) {
            clients.push_back(clientSockets[i]);
        }
    }
    return clients;
}

std::string NetworkServer::getLocalIPAddress() const {
    struct ifaddrs *ifaddrs_ptr = nullptr;
    struct ifaddrs *ifa = nullptr;
    std::string ipAddress = "127.0.0.1";
    
    if (getifaddrs(&ifaddrs_ptr) == -1) {
        return ipAddress;
    }
    
    for (ifa = ifaddrs_ptr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* addr_in = (struct sockaddr_in*)ifa->ifa_addr;
            char addr_str[INET_ADDRSTRLEN];
            
            if (inet_ntop(AF_INET, &(addr_in->sin_addr), addr_str, INET_ADDRSTRLEN)) {
                // 跳过回环地址
                std::string addr(addr_str);
                if (addr.find("127.") != 0 && addr.find("169.254") != 0) {
                    ipAddress = addr;
                    break;
                }
            }
        }
    }
    
    freeifaddrs(ifaddrs_ptr);
    return ipAddress;
}

} // namespace chessgame::network