#include <stdlib.h>

#include "hand.h"
#include "strategy.h"

Strategy::Strategy() {
  static const double init_win_prob[3][NUM_PATTERNS] = {
    {.3, .6,  0, .9,  0,  0,  0, 0, 0, 0},
    { 0, .1, .3, .5, .7, .9,  1, 1, 1, 1},
    { 0,  0,  0, .2, .4, .6, .8, 1, 1, 1},
  };
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < NUM_PATTERNS; ++j) {
      for (int r = 0; r < NUM_RANKS; ++r) {
        stats_[i][j][r].win_prob = init_win_prob[i][j];
        stats_[i][j][r].wins = 0;
        stats_[i][j][r].total = 0;
      }
    }
  }
}

void Strategy::Update(int nth, const Pattern& p, int result) {
  int r = p.back()->rank;
  if (result == 1) {
    ++stats_[nth][p.pattern()][r].wins;
  }
  ++stats_[nth][p.pattern()][r].total;
}

double Strategy::GetWinningProbability(int nth, const Pattern& p) const {
  int high_rank = p.set().back()->rank;
  return stats_[nth][p.pattern()][high_rank].win_prob;
}

void Strategy::UpdateWinningProbabilities() {
  for (int r = 0; r < NUM_RANKS; ++r) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < NUM_PATTERNS; ++j) {
        auto* stat = &stats_[i][j][r];
        if (stat->total >= 10) {
          stat->win_prob = double(stat->wins)/stat->total;
        }
      }
    }
  }
}

void Strategy::ShowWinningProbabilities() const {
  for (int r = 0; r < NUM_RANKS; ++r) {
    for (int j = 0; j < NUM_PATTERNS; ++j) {
      printf("%11s", pattern_names[j]);
    }
    printf("%11c\n", rank_symbols[r]);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < NUM_PATTERNS; ++j) {
        auto* stat = &stats_[i][j][r];
        if (stat->total >= 10) {
          printf("%11.2f", stat->win_prob);
        } else {
          printf("%11c", '-');
        }
      }
      printf("\n");
    }
  }
}

