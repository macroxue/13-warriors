#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "card.h"
#include "pattern.h"

class Strategy {
 public:
  Strategy();
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

  int num_updates;
};

#endif  // STRATEGY_H_
