#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define PORT 8080

void sendBet(SOCKET sock) {
    int betAmount;
    cout << "Введите сумму ставки: ";
    cin >> betAmount;
    cin.ignore();

    int betNetwork = htonl(betAmount);
    int sendResult = send(sock, reinterpret_cast<char*>(&betNetwork), sizeof(betNetwork), 0);
    if (sendResult == SOCKET_ERROR) {
        cerr << "Ошибка отправки ставки: " << WSAGetLastError() << endl;
    }
}

bool receiveMessage(SOCKET sock) {
    char buffer[1024] = { 0 };
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        cout << "Сообщение от сервера: " << buffer << endl;
        if (strcmp(buffer, "exit") == 0) {
            cout << "Сервер завершил соединение.\n";
            return false;
        }
    }
    else if (bytesReceived == 0) {
        cout << "Сервер закрыл соединение.\n";
        return false;
    }
    else {
        cerr << "Ошибка при получении данных: " << WSAGetLastError() << endl;
        return false;
    }
    return true;
}

bool sendMessage(SOCKET sock) {
    string userInput;
    cout << "Введите ваш ответ: ";
    getline(cin, userInput);

    int sendResult = send(sock, userInput.c_str(), userInput.length(), 0);
    if (sendResult == SOCKET_ERROR) {
        cerr << "Ошибка отправки: " << WSAGetLastError() << endl;
        return false;
    }

    if (userInput == "R" || userInput == "r") {
        sendBet(sock);
    }
    return true;
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    WSADATA wsaData;
    SOCKET sock;
    sockaddr_in serv_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка Winsock" << endl;
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета" << endl;
        WSACleanup();
        return 1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        cerr << "Ошибка подключения к серверу" << endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    if (!receiveMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    if (!sendMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    cout << "---- pre-flop ----" << endl;
    if (!receiveMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    if (!sendMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
      cout << "---- flop ----" << endl;
    if (!receiveMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    if (!sendMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    cout << "---- turn ----" << endl;
    if (!receiveMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    if (!sendMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    cout << "---- river ----" << endl;
    if (!receiveMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    if (!sendMessage(sock)) {
        closesocket(sock);
        WSACleanup();
        return 0;
    }
    closesocket(sock);
    WSACleanup();
    return 0;
}
