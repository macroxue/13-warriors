#ifndef CARD_H_
#define CARD_H_

#include <stdio.h>

enum Suit { SPADE, HEART, CLUB, DIAMOND, NUM_SUITS };

enum Rank {
  ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
  JACK, QUEEN, KING, ACE, NUM_RANKS
};

static const char suit_symbols[] = "SHCD";
static const char rank_symbols[] = "123456789TJQKA";

struct Card {
  int  suit;
  int  rank;
  bool in_use;

  void Show(FILE *fp = stdout) const {
    fprintf(fp, "%c%c ", suit_symbols[suit], rank_symbols[rank]);
  }
};

#endif  // CARD_H_
