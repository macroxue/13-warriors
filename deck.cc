#include <assert.h>
#include <stdlib.h>

#include <algorithm>

#include "deck.h"

Deck::Deck()
  : top(0) {
  for (int i = 0; i < 52; ++i) {
    cards[i].suit = i/13;
    cards[i].rank = i%13+1;
    cards[i].in_use = false;
  }
}

Card* Deck::FindCard(int suit, int rank) {
  if (rank == ONE) {
    rank = ACE;
  }
  Card* card = &cards[suit*13 + rank-1];
  assert(card->suit == suit && card->rank == rank);
  return card;
}

void Deck::Shuffle() {
  top = 0;
  for (int i = 0; i < 52; ++i) {
    std::swap(cards[i], cards[rand() % 52]);
  }
}

Card* Deck::DealOneCard() {
  return &cards[top++];
}
