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
  Card cards_[52];
  int  top_;
};

#endif  // DECK_H_
