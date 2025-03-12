#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
using namespace std;
#include "Card.h";
class Deck {
    vector<Card> cards;

public:
    Deck() {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 13; j++) {
                cards.emplace_back(Nominal(j), Mask(i));
            }
        }
        shuffleDeck();
    }

    void shuffleDeck() {
        random_device rd;
        mt19937 g(rd());
        shuffle(cards.begin(), cards.end(), g);
    }

    Card dealCard() {
        if (cards.empty()) {
            throw runtime_error("Deck is empty!");
        }
        Card topCard = cards.back();
        cards.pop_back();
        return topCard;
    }
    size_t size() {
        return cards.size();
    }
    Card& operator[](int num) {
        return cards.at(num);
    }
};