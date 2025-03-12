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
#pragma comment(lib, "ws2_32.lib") 
using namespace std;
#define PORT 8080
std::mutex betMutex;
#include "server.h"
#include "Card.h"
#include"Deck.h"
#include"Hand.h"
#include"AI.h"
std::map<SOCKET, bool> turn_checked;
std::map<SOCKET, bool> river_checked;
int main() {
    try {

        SetConsoleOutputCP(1251);
        SetConsoleCP(1251);
        WSADATA wsaData;
        SOCKET server_socket, client_socket;
        sockaddr_in server_addr, client_addr;
        int addrlen = sizeof(client_addr);
        vector<thread> threads;

        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "error Winsock" << endl;
            return 1;
        }

        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == INVALID_SOCKET) {
            cerr << "error socket" << endl;
            WSACleanup();
            return 1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(PORT);

        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
            cerr << "disconnected" << endl;
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

        if (listen(server_socket, MAX_CLIENTS) == SOCKET_ERROR) {
            cerr << "error waiting " << endl;
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

        cout << "Server waiting for clients..." << endl;
        while (clients.size() < MAX_CLIENTS) {
            client_socket = accept(server_socket, (sockaddr*)&client_addr, (socklen_t*)&addrlen);
            if (client_socket == INVALID_SOCKET) {
                cerr << "Client connection failed" << endl;
                continue;
            }

            cout << "Client connected: " << client_socket << endl;
            lock_guard<mutex> lock(clients_mutex);
            clients.push_back(client_socket);
            client_ready[client_socket] = false;

            threads.emplace_back(handle_client, client_socket);
        }
        cout << "Waiting for all clients to be ready..." << endl;
        while (!all_clients_ready()) {
            this_thread::sleep_for(chrono::seconds(2));

        }

        cout << "All clients are ready! Starting game..." << endl;

        int numPlayers = threads.size();
        int mynumber = 0;
        double bigBlind = 100;
        cout << "Enter the big blind amount: ";
        std::cin.ignore(1000, '\n');
        Deck deck;
        vector<Hand> gamers;
        Hand playerHand;
        for (size_t i = 0; i <= mynumber; i++) {
            if (i != mynumber) {
                Hand newHand(deck.dealCard(), deck.dealCard());
                gamers.push_back(newHand);
                player_hand_cards(clients[i], gamers[i].first, gamers[i].second);
            }
            else{
                Card card1 = deck.dealCard();
                cout << "Your first card: " << card1.toString() << endl;
                Card card2 = deck.dealCard();
                cout << "Your second card: " << card2.toString() << endl;
                playerHand.first=card1;
                playerHand.second = card2;
            }
        }
        for(int i=mynumber;i<numPlayers;i++){
            Hand newHand(deck.dealCard(), deck.dealCard());
            gamers.push_back(newHand);
            player_hand_cards(clients[i], gamers[i].first, gamers[i].second);
        }
     
        int nom1 = static_cast<int>(playerHand.first.nominal);
        int nom2 = static_cast<int>(playerHand.second.nominal);
        int highnom = playerHand.maks(playerHand);
        if (playerHand.first.nominal == playerHand.second.nominal || (abs(nom1 - nom2) <= 5 && (nom1 + nom2) > 16) || (playerHand.first.mask == playerHand.second.mask && (highnom >= 10))) {
            cout << "Play\n";
        }
        else {
            cout << "Fold\n";
        }
        cout << "Your hand: " << playerHand.toString() << endl;
        vector<double> bets(numPlayers);
        for (int i = 0; i < numPlayers; ++i) {
            if (i != mynumber) {
                threads.emplace_back([&](SOCKET sock, Card& c1, Card& c2) {
                    player_hand_cards(sock, c1, c2);
                    }, clients[i], std::ref(gamers[i].first), std::ref(gamers[i].second));

            }
        }

        PokerAI pokerAI;

        // Calculate and display the player's hand strength
        vector<Card> playerCards = { playerHand.first, playerHand.second };
        double playerStrength = pokerAI.evaluatePreFlop(playerHand);
        cout << "strength preflop= " << playerStrength << endl;
        // Deal three community cards (flop)
        vector<Card> communityCards;
        for (int i = 0; i < 3; i++) {
            Card card3 = deck.dealCard();
            communityCards.push_back(card3);
            cout << "Your flop cards: " << card3.toString() << endl;
        }
        cout << "Community cards: ";
        for (const auto& card : communityCards) {
            cout << card.toString() << " ";
        }
        vector<double> player_bets(numPlayers, 0); 

        for (int i = 0; i < numPlayers; ++i) {
                threads.emplace_back([=](SOCKET sock, Card& c1, Card& c2, Card& c3, double betCopy) {
                    flop_cards(sock, c1, c2, c3, betCopy);
                    }, clients[i], std::ref(communityCards[0]), std::ref(communityCards[1]), std::ref(communityCards[2]), player_bets[i]);
        }
        std::cin.ignore(1000, '\n');
        numPlayers = player_bets.size();
        playerStrength = pokerAI.evaluatePostFlop(playerCards, communityCards, playerStrength);
        cout << "Player hand strength: " << playerStrength << endl;

        double opponentStrength = 0.0;
        for (double bet : bets) {
            opponentStrength += pokerAI.calculateOpponentStrength(bet);
        }
        opponentStrength = opponentStrength /bets.size();
        cout << "Opponent estimated strength: " << opponentStrength << endl;
        int players = bets.size();
        cout << players<<" players we got\n";
        cout << clients.size() << " we got \n";
        for (int i = 0; i < players; i++) {
            int enemynumber = i;
            double enemyblind = bets[i];
            string decision = pokerAI.makeDecision(playerStrength, opponentStrength, communityCards, deck, mynumber, enemynumber, enemyblind, bigBlind, 3);
            cout << "Poker AI decision: " << decision << endl;
        }
        string card4Input;
        Card card4 = deck.dealCard();
        communityCards.push_back(card4);

        cout << "Community cards: ";
        for (const auto& card : communityCards) {
            cout << card.toString() << " ";
        }
        cout << endl;
        cout << numPlayers;
        for (int i = 0; i < numPlayers; ++i) {
            threads.emplace_back([=](SOCKET sock, Card& c1, Card& c2, Card& c3,Card&c4, double betCopy) {
                turn_cards(sock, c1, c2, c3,c4, betCopy);
                }, clients[i], std::ref(communityCards[0]), std::ref(communityCards[1]), std::ref(communityCards[2]),
                    std::ref(communityCards[3]), player_bets[i]);
        }
        playerStrength = pokerAI.evaluatePreFlop(playerHand);
        for (double bet : bets) {
            opponentStrength += pokerAI.calculateOpponentStrength(bet);
        }
        opponentStrength = opponentStrength * bets.size();
        playerStrength = pokerAI.evaluatePostFlop(playerCards, communityCards, playerStrength);
        cout << "Player hand strength: " << playerStrength << endl;

        for (int i = 0; i < player_bets.size(); i++) {
            int enemynumber = i;
            double enemyblind =(player_bets[i]);
            cin >> enemynumber >> enemyblind;
            string decision = pokerAI.makeDecision(playerStrength, opponentStrength, communityCards, deck, mynumber, enemynumber, enemyblind, bigBlind, 4);
            cout << "Poker AI decision: " << decision << endl;
        }
        //river
        std::cin.ignore(1000, '\n');
        string card5Input;
        Card card5 = deck.dealCard();
        communityCards.push_back(card5);
        cout << "Your river card: " << card5.toString() << endl;
        cout << "Community cards: ";
        for (const auto& card : communityCards) {
            cout << card.toString() << " ";
        }
        cout << endl;
        for (int i = 0; i < numPlayers; ++i) {
            threads.emplace_back([=](SOCKET sock, Card& c1, Card& c2, Card& c3,Card&c4,Card&c5,double betCopy) {
                river_cards(sock, c1, c2, c3,c4,c5, betCopy);
                }, clients[i], std::ref(communityCards[0]), std::ref(communityCards[1]), std::ref(communityCards[2]), 
                    std::ref(communityCards[3]), std::ref(communityCards[4]), player_bets[i]);
        }
        numPlayers = player_bets.size();
        playerStrength = pokerAI.evaluatePreFlop(playerHand);
        for (double bet : bets) {
            opponentStrength += pokerAI.calculateOpponentStrength(bet);
        }
        opponentStrength = opponentStrength / bets.size();
        playerStrength = pokerAI.evaluatePostFlop(playerCards, communityCards, playerStrength);
        cout << "Player hand strength: " << playerStrength << endl;
        
        cout << "How many players we got\n";
        for (int i = 0; i < numPlayers; i++) {
            cout << "Enter enemy number and enemy blind\n";
            int enemynumber = 0;
            double enemyblind = player_bets[0];
            string decision = pokerAI.makeDecisionriver(playerStrength, opponentStrength, communityCards, deck, mynumber, enemynumber, enemyblind, bigBlind, 4);
            cout << "Poker AI decision: " << decision << endl;
        }

        for (SOCKET client : clients) {
            closesocket(client);
        }
        closesocket(server_socket);
        WSACleanup();
        for (auto& t : threads) {
            if (t.joinable()) {
                t.join();
            }
        }

    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }

    return 0;
}