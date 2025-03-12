#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
using namespace std;
#include "Card.h";
class Hand {
public:
    Card first;
    Card second;

    Hand(Card a, Card b) : first(a), second(b) {}
    string toString() const {
        return first.toString() + ", " + second.toString();
    }
    Hand() :first(), second() {}
    int maks(Hand& h) {
        if (static_cast<int>(h.first.nominal) >= static_cast<int>(h.second.nominal)) {
            return static_cast<int>(h.first.nominal);

        }
        else {
            return static_cast<int>(h.second.nominal);
        }
    }
};