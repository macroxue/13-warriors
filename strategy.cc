#include <stdlib.h>

#include "hand.h"
#include "strategy.h"

Strategy::Strategy()
  : num_updates(0) {
  static const double init_win_prob[3][NUM_PATTERNS] = {
    {.3, .6,  0,  1,  0,  0,  0, 0, 0, 0},
    { 0, .1, .4, .7, .8, .9,  1, 1, 1, 1},
    { 0,  0, .1, .2, .3, .5, .8, 1, 1, 1},
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
  ++num_updates;
  if (num_updates % 100 == 0) {
    Refresh();
  }
}

double Strategy::GetWinningProbability(int nth, const Pattern& p) const {
  int high_rank = p.set().back()->rank;
  return stats_[nth][p.pattern()][high_rank].win_prob;
}

void Strategy::Refresh() {
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

void Strategy::Show() const {
  const int name_width = 14;
  const int data_width = 5;
  for (int p = 0; p < NUM_PATTERNS; ++p) {
    printf("%*s:", name_width, pattern_names[p]);
    for (int r = 1; r < NUM_RANKS; ++r) {
      printf("%*c", data_width, rank_symbols[r]);
    }
    printf("\n");
    for (int s = 0; s < 3; ++s) {
      printf("%*d:", name_width, s+1);
      for (int r = 1; r < NUM_RANKS; ++r) {
        auto* stat = &stats_[s][p][r];
        if (stat->total >= 10) {
          printf("%*.0f%%", data_width-1, stat->win_prob*100);
        } else {
          printf("%*c", data_width, '-');
        }
      }
      printf("\n");
    }
  }
}

