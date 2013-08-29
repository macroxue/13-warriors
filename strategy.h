#ifndef STRATEGY_H_
#define STRATEGY_H_

#include "card.h"
#include "pattern.h"

class Strategy {
 public:
  void Learn(int seed, int rounds, int update_cycle);
  void Update(int nth, const Set& set1, int p1, const Set& set2, int p2, int result);

  double GetWinningProbability(int nth, int pattern, const Set& set) const;
  void ShowWinningProbabilities();

 private:
  struct Stat {
    double win_prob;
    int    wins;
    int    total;
  } stats[3][NUM_PATTERNS][NUM_RANKS];
};

#endif  // STRATEGY_H_
