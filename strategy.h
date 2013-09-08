#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "card.h"
#include "pattern.h"

class Strategy {
 public:
  Strategy(int min_1st_rank = NUM_RANKS);
  void Update(int nth, const Pattern& p, int result);

  double GetWinningProbability(int nth, const Pattern& p) const;
  void Show() const;

 private:
  void Refresh();

  struct Stat {
    double win_prob;
    int    wins;
    int    total;
  };

  Stat stats_[3][NUM_PATTERNS][NUM_RANKS];
  Stat junk0_[NUM_RANKS][NUM_RANKS];
  Stat flush2_[NUM_RANKS][NUM_RANKS];

  int num_updates_;
  int min_1st_rank_;  // Minimum 1st rank for stats of 2nd rank.
};

#endif  // STRATEGY_H_
