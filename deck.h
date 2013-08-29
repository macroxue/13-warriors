#ifndef DECK_H_
#define DECK_H_

#include "card.h"

class Deck {
 public:
  Deck();
  Card* FindCard(int suit, int rank);
  void Shuffle();
  Card* DealOneCard();

 private:
  Card cards[52];
  int  top;
};

#endif  // DECK_H_
