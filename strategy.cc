#include <stdlib.h>

#include "hand.h"
#include "strategy.h"

Strategy::Strategy(int min_1st_rank)
  : num_updates_(0), min_1st_rank_(min_1st_rank) {
  static const int win_percent[NUM_PATTERNS][3][NUM_RANKS] = {
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   JUNK
      {  0,  0,  0,  0,  0,  0,  1,  1,  2,  2,  4,  7, 14, 33 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   PAIR
      {  0, 46, 48, 50, 51, 54, 56, 60, 63, 68, 74, 81, 89, 97 },
      {  0,  2,  3,  4,  4,  5,  7,  8, 10, 12, 15, 19, 24, 33 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  2,  3 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   TWO_PAIRS
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0, 36, 37, 38, 40, 44, 46, 49, 54, 57, 62, 64,  0 },
      {  0,  0,  2,  3,  4,  4,  6,  7,  8, 10, 11, 13, 13,  0 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   TRIPLE
      {  0, 99, 99,100,100,100,100,100,100,100,100,100,100,100 },
      {  0, 63, 65, 69, 71, 72, 73, 73, 73, 74, 74, 75, 75, 75 },
      {  0, 11, 12, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   STRAIGHT
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0, 77, 78, 81, 83, 85, 87, 88, 90, 91, 92 },
      {  0,  0,  0,  0, 16, 17, 20, 22, 24, 26, 28, 32, 33, 36 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   FLUSH
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0, 93, 93, 93, 93, 94, 95, 97, 98 },
      {  0,  0,  0,  0,  0,  0, 36, 36, 37, 38, 40, 44, 49, 61 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   FULL_HOUSE
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0, 98, 98, 99, 99, 99,100,100,100,100,100,100,100,100 },
      {  0, 64, 67, 70, 71, 73, 75, 77, 80, 82, 85, 88, 91, 94 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   FOUR_OF_A_KIND
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,100,100,100,100,100,100,100,100,100,100,100,100,100 },
      {  0, 93, 94, 95, 95, 96, 96, 96, 97, 97, 98, 98, 98, 98 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   STRAIGHT_FLUSH
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,100,100,100,100,100,100,100,100,100,  0 },
      {  0,  0,  0,  0, 98, 98, 99, 99, 99, 99, 99, 99,100,  0 } },
    { // 1   2   3   4   5   6   7   8   9   T   J   Q   K   A   ROYAL_FLUSH
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,100 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,100 } }
  };
  for (int s = 0; s < 3; ++s) {
    for (int p = 0; p < NUM_PATTERNS; ++p) {
      for (int r = 0; r < NUM_RANKS; ++r) {
        auto* stat = &stats_[s][p][r];
        stat->win_prob = win_percent[p][s][r] / 100.0;
        stat->wins = 0;
        stat->total = 0;
      }
    }
  }
  for (int r = min_1st_rank_; r < NUM_RANKS; ++r) {
    for (int r2 = 1; r2 < NUM_RANKS; ++r2) {
      junk0_[r][r2] = stats_[0][JUNK][r];
      flush2_[r][r2] = stats_[2][FLUSH][r];
    }
  }
}

void Strategy::Update(int nth, const Pattern& p, int result) {
  int r = p.back()->rank;
  if (result == 1) {
    ++stats_[nth][p.pattern()][r].wins;
  }
  ++stats_[nth][p.pattern()][r].total;

  if (r >= min_1st_rank_) {
    int r2 = p[p.size()-2]->rank;
    if (p.pattern() == JUNK && nth == 0) {
      if (result == 1) {
        ++junk0_[r][r2].wins;
      }
      ++junk0_[r][r2].total;
    }
    if (p.pattern() == FLUSH && nth == 2) {
      if (result == 1) {
        ++flush2_[r][r2].wins;
      }
      ++flush2_[r][r2].total;
    }
  }

  ++num_updates_;
  if (num_updates_ % 100 == 0) {
    Refresh();
  }
}

double Strategy::GetWinningProbability(int nth, const Pattern& p) const {
  int r = p.back()->rank;

  if (r >= min_1st_rank_) {
    int r2 = p[p.size()-2]->rank;
    if (p.pattern() == JUNK && nth == 0) {
      return junk0_[r][r2].win_prob;
    }
    if (p.pattern() == FLUSH && nth == 2) {
      return flush2_[r][r2].win_prob;
    }
  }

  return stats_[nth][p.pattern()][r].win_prob;
}

void Strategy::Refresh() {
  for (int r = 0; r < NUM_RANKS; ++r) {
    for (int s = 0; s < 3; ++s) {
      for (int p = 0; p < NUM_PATTERNS; ++p) {
        auto* stat = &stats_[s][p][r];
        if (stat->total >= 10) {
          stat->win_prob = double(stat->wins)/stat->total;
        }
      }
    }
  }

  for (int r = min_1st_rank_; r < NUM_RANKS; ++r) {
    for (int r2 = 0; r2 < NUM_RANKS; ++r2) {
      if (junk0_[r][r2].total >= 10) {
        junk0_[r][r2].win_prob = double(junk0_[r][r2].wins)/junk0_[r][r2].total;
      }
      if (flush2_[r][r2].total >= 10) {
        flush2_[r][r2].win_prob = double(flush2_[r][r2].wins)/flush2_[r][r2].total;
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
        auto& stat = stats_[s][p][r];
        if (stat.total >= 10) {
          printf("%*.0f%%", data_width-1, stat.win_prob*100);
        } else {
          printf("%*c", data_width, '-');
        }
      }
      printf("\n");
    }
    if (p == JUNK || p == FLUSH) {
      for (int r = min_1st_rank_; r < NUM_RANKS; ++r) {
        printf("%*c-?:", name_width-2, rank_symbols[r]);
        for (int r2 = 1; r2 < NUM_RANKS; ++r2) {
          const Stat* stat;
          if (p == JUNK)
            stat = &junk0_[r][r2];
          else
            stat = &flush2_[r][r2];
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
}

