#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "card.h"
#include "pattern.h"

class Strategy {
 public:
  Strategy();
  void Learn(int seed, int rounds, int update_cycle);
  void Update(int nth, const Pattern& p1, const Pattern& p2, int result);

  double GetWinningProbability(int nth, const Pattern& p) const;
  void UpdateWinningProbabilities();
  void ShowWinningProbabilities();

 private:
  struct Stat {
    double win_prob;
    int    wins;
    int    total;
  };

  Stat stats_[3][NUM_PATTERNS][NUM_RANKS];
};

#endif  // STRATEGY_H_
