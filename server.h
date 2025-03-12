#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <limits>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread> 
#include <mutex>
#include <map>
#include"Card.h"
extern std::map<SOCKET, bool> player_checked;
extern std::map<SOCKET, bool> flop_checked;
extern std::map<SOCKET, bool> turn_checked;
extern std::map<SOCKET, bool> river_checked;
extern std::mutex clients_mutex;
const int MAX_CLIENTS = 2;
std::vector<SOCKET> clients;
std::map<SOCKET, bool> client_ready;
std::mutex clients_mutex;
std::map<SOCKET, bool> player_checked;
std::map<SOCKET, bool> flop_checked;
void handle_client(SOCKET client_socket) {
    char buffer[256];
    int bytesReceived;

    const char* question = "READY? (y/n)\n";
    send(client_socket, question, strlen(question), 0);

    bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::cout << "Client response: " << buffer << std::endl;

        if (buffer[0] == 'y' || buffer[0] == 'Y') {
            std::cout << "Client is ready!" << std::endl;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                client_ready[client_socket] = true;
            }
        }
        else {
            std::cout << "Client is not ready!" << std::endl;
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                client_ready[client_socket] = false;
            }
        }
    }
}
bool all_clients_ready() {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& pair : client_ready) {
        if (!pair.second) {
            return false;
        }
    }
    return true; 
}
void player_hand_cards(SOCKET client_socket, Card& f, Card& s) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (player_checked[client_socket]) {
        return;
    }

    player_checked[client_socket] = true; 

    char buffer[256];
    int bytesReceived;
    std::string message = "Your cards: " + f.toString() + " | " + s.toString() + " ready? Y/N\n";

    int bytesSent=send(client_socket, message.c_str(), message.length(), 0);

    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
        return;
    }

    bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived == SOCKET_ERROR) {
        std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
        return;
    }

    buffer[bytesReceived] = '\0';
}
void flop_cards(SOCKET client_socket, Card& f, Card& s, Card& t, double& bets) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (flop_checked[client_socket]) {
        return;
    }
    flop_checked[client_socket] = true;

    std::string message = "Flop Cards: " + f.toString() + " | " + s.toString() + " | " + t.toString() +
        " Play? (C - check, F - fold, R - raise)\n";

    if (send(client_socket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return;
    }

    char buffer[256];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, sizeof(buffer)); 
        bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cerr << "Client disconnected or error: " << WSAGetLastError() << std::endl;
            return;
        }

        buffer[bytesReceived] = '\0';
        char choice = buffer[0];

        if (choice == 'C' || choice == 'c') {
            bets = 1;
            break;
        }
        else if (choice == 'F' || choice == 'f') {
            bets = 0;
            break;
        }
        else if (choice == 'R' || choice == 'r') {
            int betAmount;
            int received = recv(client_socket, (char*)&betAmount, sizeof(int), MSG_WAITALL);

            if (received != sizeof(int)) {
                std::cerr << "Error receiving bet amount: " << WSAGetLastError() << std::endl;
                continue;
            }

            betAmount = ntohl(betAmount); 
            if (betAmount < 0) {
                std::string errorMsg = "Invalid bet amount! Try again.\n";
                send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
                continue;
            }

            std::cout << "Received bet: " << betAmount << std::endl;
            bets = static_cast<double>(betAmount) / 100.0;
            break;
        }
        else {
            std::string errorMsg = "Invalid input! Try again.\n";
            send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
        }
    }
}
void turn_cards(SOCKET client_socket, Card& f, Card& s, Card& t,Card &fo, double& bets) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (turn_checked[client_socket]) {
        return;
    }
    turn_checked[client_socket] = true;
    std::string message = "turn Cards: " + f.toString() + " | " + s.toString() + " | " + t.toString() +" |"+fo.toString()+
        " Play? (C - check, F - fold, R - raise)\n";

    if (send(client_socket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return;
    }

    char buffer[256];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, sizeof(buffer)); 
        bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cerr << "Client disconnected or error: " << WSAGetLastError() << std::endl;
            return;
        }

        buffer[bytesReceived] = '\0';
        char choice = buffer[0];

        if (choice == 'C' || choice == 'c') {
            bets = 1;
            break;
        }
        else if (choice == 'F' || choice == 'f') {
            bets = 0;
            break;
        }
        else if (choice == 'R' || choice == 'r') {
            int betAmount;
            int received = recv(client_socket, (char*)&betAmount, sizeof(int), MSG_WAITALL);

            if (received != sizeof(int)) {
                std::cerr << "Error receiving bet amount: " << WSAGetLastError() << std::endl;
                continue;
            }

            betAmount = ntohl(betAmount);
            if (betAmount < 0) {
                std::string errorMsg = "Invalid bet amount! Try again.\n";
                send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
                continue;
            }

            std::cout << "Received bet: " << betAmount << std::endl;
            bets = static_cast<double>(betAmount) / 100.0;
            break;
        }
        else {
            std::string errorMsg = "Invalid input! Try again.\n";
            send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
        }
    }
}
void river_cards(SOCKET client_socket, Card& f, Card& s, Card& t, Card& fo,Card&r, double& bets) {
    std::lock_guard<std::mutex> lock(clients_mutex);

    if (river_checked[client_socket]) {
        return;
    }
    river_checked[client_socket] = true;

    std::string message = "turn Cards: " + f.toString() + " | " + s.toString() + " | " + t.toString() + 
        " |" + fo.toString() +r.toString()+" | "+
        " Play? (C - check, F - fold, R - raise)\n";

    if (send(client_socket, message.c_str(), message.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
        return;
    }

    char buffer[256];
    int bytesReceived;

    while (true) {
        memset(buffer, 0, sizeof(buffer)); 
        bytesReceived = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0) {
            std::cerr << "Client disconnected or error: " << WSAGetLastError() << std::endl;
            return;
        }

        buffer[bytesReceived] = '\0';
        char choice = buffer[0];

        if (choice == 'C' || choice == 'c') {
            bets = 1;
            break;
        }
        else if (choice == 'F' || choice == 'f') {
            bets = 0;
            break;
        }
        else if (choice == 'R' || choice == 'r') {
            int betAmount;
            int received = recv(client_socket, (char*)&betAmount, sizeof(int), MSG_WAITALL);

            if (received != sizeof(int)) {
                std::cerr << "Error receiving bet amount: " << WSAGetLastError() << std::endl;
                continue;
            }

            betAmount = ntohl(betAmount);
            if (betAmount < 0) {
                std::string errorMsg = "Invalid bet amount! Try again.\n";
                send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
                continue;
            }

            std::cout << "Received bet: " << betAmount << std::endl;
            bets = static_cast<double>(betAmount) / 100.0;
            break;
        }
        else {
            std::string errorMsg = "Invalid input! Try again.\n";
            send(client_socket, errorMsg.c_str(), errorMsg.size(), 0);
        }
    }
}