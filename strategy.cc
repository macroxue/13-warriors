#include <stdlib.h>

#include "hand.h"
#include "strategy.h"

void Strategy::Learn(int seed, int rounds, int update_cycle) {
  static const double init_win_prob[3][NUM_PATTERNS] = {
    {.3, .6,  0, .9,  0,  0,  0, 0, 0, 0},
    { 0, .1, .3, .5, .7, .9,  1, 1, 1, 1},
    { 0,  0,  0, .2, .4, .6, .8, 1, 1, 1},
  };
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < NUM_PATTERNS; ++j) {
      for (int r = 0; r < NUM_RANKS; ++r) {
        stats[i][j][r].win_prob = init_win_prob[i][j];
        stats[i][j][r].wins = 0;
        stats[i][j][r].total = 0;
      }
    }
  }

  printf("SEED:\t\t%d\n", seed);
  srand(seed);

  for (int r = 0; r < rounds; ++r) {
    for (int f = 0; f < update_cycle; ++f) {
      Deck deck;
      deck.Shuffle();

      Hand hands[4];
      for (int i = 0; i < 4; ++i) {
        hands[i].set_strategy(this);
        hands[i].DealFrom(&deck);
        hands[i].ArrangeSets();
      }

      for (int i = 0; i < 3; ++i) {
        for (int j = i+1; j < 4; j++) {
          hands[i].Match(hands[j]);
        }
      }
#if 0
      for (int i = 0; i < 4; ++i) {
        hands[i].Show();
      }
#endif
    }
  }
}

void Strategy::Update(int nth, const Set& set1, int p1, const Set& set2, int p2, int result) {
  int r1 = set1.back()->rank;
  int r2 = set2.back()->rank;
  if (result == 1) {
    ++stats[nth][p1][r1].wins;
  } else if (result == -1) {
    ++stats[nth][p2][r2].wins;
  }
  ++stats[nth][p1][r1].total;
  ++stats[nth][p2][r2].total;
}

double Strategy::GetWinningProbability(int nth, int pattern, const Set& set) const {
  int high_rank = set.back()->rank;
  return stats[nth][pattern][high_rank].win_prob;
}

void Strategy::ShowWinningProbabilities() {
  for (int r = 0; r < NUM_RANKS; ++r) {
    for (int j = 0; j < NUM_PATTERNS; ++j) {
      printf("%11s", pattern_names[j]);
    }
    printf("%11c\n", rank_symbols[r]);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < NUM_PATTERNS; ++j) {
        auto* stat = &stats[i][j][r];
        if (stat->total >= 10) {
          stat->win_prob = double(stat->wins)/stat->total;
          printf("%11.2f", stat->win_prob);
        } else {
          printf("%11c", '-');
        }
      }
      printf("\n");
    }
  }
}

