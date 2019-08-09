#include <assert.h>
#include <stdlib.h>

#include <algorithm>

#include "deck.h"

Deck::Deck()
  : top_(0) {
  for (int i = 0; i < 52; ++i) {
    cards_[i].suit = i/13;
    cards_[i].rank = i%13+1;
    cards_[i].in_use = false;
  }
}

Card* Deck::FindCard(int suit, int rank) {
  if (rank == ONE) {
    rank = ACE;
  }
  Card* card = &cards_[suit*13 + rank-1];
  assert(card->suit == suit && card->rank == rank);
  return card;
}

void Deck::Shuffle() {
  top_ = 0;
  for (int i = 0; i < 52; ++i) {
    std::swap(cards_[i], cards_[rand() % (52-i) + i]);
  }
}

Card* Deck::DealOneCard() {
  return &cards_[top_++];
}
