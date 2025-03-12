#pragma once
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
using namespace std;
enum class Mask { spades, clubs, hearts, diamonds };
enum class Nominal {
    two, three, four, five, six, seven, eight, nine, ten,
    jack, queen, king, ace
};

class Card {
public:
    Mask mask;
    Nominal nominal;

    Card(Nominal q, Mask w) : nominal(q), mask(w) {}
    Card() : nominal(Nominal::two), mask(Mask::spades) {}

    string toString() const {
        string nominals[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10",
                              "J", "Q", "K", "A" };
        string masks[] = { "Spades", "Clubs", "Hearts", "Diamonds" };
        return nominals[static_cast<int>(nominal)] + " of " + masks[static_cast<int>(mask)];
    }
    void enter() {
        cout << "enter cards nominal mask \n";

        int m = 0;
        int n = 0;

    }
    static Card fromString(const string& input) {
        size_t spacePos = input.find(' ');
        if (spacePos == string::npos) {
            throw invalid_argument("Invalid input format. Use format like '10 Hearts'.");
        }

        string nominalStr = input.substr(0, spacePos);
        string maskStr = input.substr(spacePos + 1);

        Nominal nominal;
        if (nominalStr == "2") nominal = Nominal::two;
        else if (nominalStr == "3") nominal = Nominal::three;
        else if (nominalStr == "4") nominal = Nominal::four;
        else if (nominalStr == "5") nominal = Nominal::five;
        else if (nominalStr == "6") nominal = Nominal::six;
        else if (nominalStr == "7") nominal = Nominal::seven;
        else if (nominalStr == "8") nominal = Nominal::eight;
        else if (nominalStr == "9") nominal = Nominal::nine;
        else if (nominalStr == "10") nominal = Nominal::ten;
        else if (nominalStr == "J") nominal = Nominal::jack;
        else if (nominalStr == "Q") nominal = Nominal::queen;
        else if (nominalStr == "K") nominal = Nominal::king;
        else if (nominalStr == "A") nominal = Nominal::ace;
        else throw invalid_argument("Invalid card nominal: " + nominalStr);
        Mask mask;
        if (maskStr == "S") mask = Mask::spades;
        else if (maskStr == "C") mask = Mask::clubs;
        else if (maskStr == "H") mask = Mask::hearts;
        else if (maskStr == "D") mask = Mask::diamonds;
        else throw invalid_argument("Invalid card mask: " + maskStr);

        return Card(nominal, mask);
    }
};